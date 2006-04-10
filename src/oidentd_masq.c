/*
** oidentd_masq.c - oidentd IP masquerading handler.
** Copyright (C) 1998-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef HAVE_LIBUDB
#	include <udb.h>
#endif

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_missing.h>
#include <oidentd_inet_util.h>
#include <oidentd_masq.h>
#include <oidentd_options.h>

struct sockaddr_storage proxy;

#ifdef MASQ_SUPPORT

static sigjmp_buf timebuf;
static int fsock;

in_port_t fwdport;

extern char *ret_os;

static void fwd_alarm(int sig) __noreturn;
static bool blank_line(const char *buf);

/*
** Returns true if the buffer contains only
** blank characters (spaces and/or tabs).  Returns
** false otherwise.
*/

static bool blank_line(const char *buf) {
	const char *p;

	for (p = buf ; *p != '\0' ; p++) {
		if (*p != ' ' && *p != '\t')
			return (false);
	}

	return (true);
}

/*
** Parse the masquerading map file.
** Returns 0 on success, -1 on failure.
*/

int find_masq_entry(struct sockaddr_storage *host,
					char *user,
					size_t user_len,
					char *os,
					size_t os_len)
{
	FILE *fp;
	struct sockaddr_storage addr;
	u_int32_t line_num;
	char buf[4096];

#ifdef HAVE_LIBUDB
	if (opt_enabled(USEUDB)) {
		struct udb_ip_user ibuf;
		struct sockaddr_storage hostaddr;
		char ipbuf[MAX_IPLEN];

		memcpy(&hostaddr, host, sizeof(hostaddr));

		get_ip(&hostaddr, ipbuf, sizeof(ipbuf));

		debug("[%s] UDB lookup...", ipbuf);

		if (udb_ip_get(SIN4(&hostaddr), &ibuf)) {
			get_ip(&hostaddr, ipbuf, sizeof(ipbuf));
			xstrncpy(user, ibuf.username, user_len);
			xstrncpy(os, ret_os, os_len);

			o_log(NORMAL, "Successful UDB lookup: %s : %s", ipbuf, user);
			return (0);
		}
	}
#endif

	fp = fopen(MASQ_MAP, "r");
	if (fp == NULL) {
		if (errno != EEXIST)
			debug("fopen: %s: %s", MASQ_MAP, strerror(errno));
		return (-1);
	}

	line_num = 0;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		struct sockaddr_storage stemp;
		char *p, *temp;

		++line_num;
		p = strchr(buf, '\n');
		if (p == NULL) {
			debug("[%s:%d] Long line", MASQ_MAP, line_num);
			goto failure;
		}
		*p = '\0';

		if (buf[0] == '#')
			continue;

		if (blank_line(buf) == true)
			continue;

		p = strchr(buf, '\r');
		if (p != NULL)
			*p = '\0';

		p = strtok(buf, " \t");
		if (p == NULL) {
			debug("[%s:%d] Missing address parameter", MASQ_MAP, line_num);
			goto failure;
		}

		temp = strchr(p, '/');
		if (temp != NULL)
			*temp++ = '\0';

		if (get_addr(p, &stemp) == -1) {
			debug("[%s:%d] Invalid address: %s", MASQ_MAP, line_num, p);
			goto failure;
		}

		sin_copy(&addr, &stemp);

		if (stemp.ss_family == AF_INET && temp != NULL) {
			in_addr_t mask, mask2;
			char *end;

			mask = strtoul(temp, &end, 10);

			if (*end != '\0') {
				if (get_addr(temp, &stemp) == -1) {
					debug("[%s:%d] Invalid address: %s",
						MASQ_MAP, line_num, temp);

					goto failure;
				}

				mask2 = SIN4(&stemp)->sin_addr.s_addr;
			} else {
				if (mask < 1 || mask > 31) {
					debug("[%s:%d] Invalid mask: %s",
						MASQ_MAP, line_num, temp);

					goto failure;
				}

				mask2 = htonl(~((1 << (32 - mask)) - 1));
			}

			SIN4(&addr)->sin_addr.s_addr &= mask2;
			SIN4(host)->sin_addr.s_addr &= mask2;
		}

		if (sin_equal(&addr, host) == false)
			continue;

		p = strtok(NULL, " \t");
		if (p == NULL) {
			debug("[%s:%d] Missing user parameter", MASQ_MAP, line_num);
			goto failure;
		}

		if (strlen(p) >= user_len) {
			debug("[%s:%d] username too long (limit is %d)",
				MASQ_MAP, line_num, user_len);

			goto failure;
		}

		xstrncpy(user, p, user_len);

		p = strtok(NULL, " \t");
		if (p == NULL) {
			debug("[%s:%d] Missing OS parameter", MASQ_MAP, line_num);

			goto failure;
		}

		if (strlen(p) >= os_len) {
			debug("[%s:%d] OS name too long (limit is %d)",
				MASQ_MAP, line_num, os_len);

			goto failure;
		}

		xstrncpy(os, p, os_len);

		fclose(fp);
		return (0);
	}

failure:
	fclose(fp);
	return (-1);
}

/*
** Forward an ident request to another machine, return the response to the
** client that has connected to us and requested it.
*/

int fwd_request(	int sock,
					in_port_t real_lport,
					in_port_t masq_lport,
					in_port_t real_fport,
					in_port_t masq_fport,
					struct sockaddr_storage *mrelay)
{
	char ipbuf[MAX_IPLEN];
	char user[512];
	char buf[1024];

	fsock = socket(mrelay->ss_family, SOCK_STREAM, 0);
	if (fsock == -1) {
		debug("socket: %s", strerror(errno));
		return (-1);
	}

	sin_set_port(fwdport, mrelay);

	if (sigsetjmp(timebuf, 1) != 0) {
		debug("sigsetjmp: %s", strerror(errno));
		return (-1);
	}

	signal(SIGALRM, fwd_alarm);

	/*
	** Five seconds should be plenty, seeing as we're forwarding to a machine
	** on a local network.
	*/

	alarm(5);

	if (connect(fsock, (struct sockaddr *) mrelay, sin_len(mrelay)) != 0) {
		get_ip(mrelay, ipbuf, sizeof(ipbuf));

		debug("connect to %s:%d: %s",
			ipbuf, ntohs(sin_port(mrelay)), strerror(errno));
		goto out_fail;
	}

	if (sockprintf(fsock, "%d , %d\r\n", masq_lport, masq_fport) < 1) {
		debug("write: %s", strerror(errno));
		goto out_fail;
	}

	if (sock_read(fsock, buf, sizeof(buf)) < 1) {
		debug("read(%d): %s\n", fsock, strerror(errno));
		goto out_fail;
	}

	/*
	** We don't want to timeout once we're finished processing the request.
	*/

	alarm(0);
	close(fsock);

	get_ip(mrelay, ipbuf, sizeof(ipbuf));

	if (sscanf(buf, "%*d , %*d : USERID :%*[^:]:%511s", user) != 1) {
		char *p = strchr(buf, '\r');

		if (p != NULL)
			*p = '\0';

		debug("[%s] Forwarding response: \"%s\"", ipbuf, buf);
		return (-1);
	}

	sockprintf(sock, "%d , %d : USERID : %s : %s\r\n",
		real_lport, real_fport, ret_os, user);

	o_log(NORMAL,
		"[%s] Successful lookup (by forward): %d (%d) , %d (%d) : %s",
		ipbuf, real_lport, masq_lport, real_fport, masq_fport, user);

	return (0);

out_fail:
	alarm(0);
	close(fsock);
	return (-1);
}

/*
** Handle the timeout of a forward request.
*/

static void fwd_alarm(int sig) {
	o_log(NORMAL, "Forward timed out");
	close(fsock);
	siglongjmp(timebuf, sig);
}

#else

/*
** Define a stub masq function.
*/

int masq(	int sock __notused,
			in_port_t lport __notused,
			in_port_t fport __notused,
			struct sockaddr_storage *local __notused,
			struct sockaddr_storage *remote __notused)
{
	return (-1);
}

#endif
