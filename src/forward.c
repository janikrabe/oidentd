/*
** forward.c - oidentd request forwarding.
** Copyright (c) 1998-2006 Ryan McCabe <ryan@numb.org>
** Copyright (c) 2018      Jan Steffens <jan.steffens@gmail.com>
** Copyright (c) 2018-2019 Janik Rabe  <oidentd@janikrabe.com>
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include "oidentd.h"
#include "util.h"
#include "missing.h"
#include "inet_util.h"
#include "options.h"
#include "forward.h"

static sigjmp_buf timebuf;
static int fsock;
static void fwd_alarm(int sig) __noreturn;

/*
** Make an ident request to another machine and return its response,
** if the request was successful.
*/

int forward_request(const struct sockaddr_storage *host,
					in_port_t port,
					in_port_t lport,
					in_port_t fport,
					char *reply,
					size_t len)
{
	struct sockaddr_storage addr;
	char ipbuf[MAX_IPLEN];
	char user[512];
	char buf[1024];

	sin_copy(&addr, host);
	sin_set_port(htons(port), &addr);

	fsock = socket(addr.ss_family, SOCK_STREAM, 0);
	if (fsock == -1) {
		debug("socket: %s", strerror(errno));
		return -1;
	}

	if (sigsetjmp(timebuf, 1) != 0) {
		debug("sigsetjmp: %s", strerror(errno));
		return -1;
	}

	signal(SIGALRM, fwd_alarm);

	/*
	** Five seconds should be plenty, seeing as we're forwarding to a machine
	** on a local network.
	*/

	alarm(5);

	if (connect(fsock, (struct sockaddr *) &addr, (socklen_t) sin_len(&addr)) != 0) {
		get_ip(&addr, ipbuf, sizeof(ipbuf));
		debug("connect to %s:%d: %s",
			ipbuf, ntohs(sin_port(&addr)), strerror(errno));
		goto out_fail;
	}

	if (sockprintf(fsock, "%d,%d\r\n", lport, fport) < 1) {
		debug("write: %s", strerror(errno));
		goto out_fail;
	}

	if (!sock_read(fsock, buf, sizeof(buf))) {
		debug("read(%d): %s", fsock, strerror(errno));
		goto out_fail;
	}

	/*
	** Don't time out once we've finished processing the request.
	*/

	alarm(0);
	close(fsock);

	if (sscanf(buf, "%*d , %*d : USERID :%*[^:]:%511s", user) != 1) {
		char *p = strchr(buf, '\r');

		if (p)
			*p = '\0';

		get_ip(&addr, ipbuf, sizeof(ipbuf));
		debug("[%s] Remote response: \"%s\"", ipbuf, buf);
		return -1;
	}

	xstrncpy(reply, user, len);
	return 0;

out_fail:
	alarm(0);
	close(fsock);
	return -1;
}

/*
** Handle the timeout of a forward request.
*/

static void fwd_alarm(int sig) {
	o_log(LOG_INFO, "Forward timed out");
	close(fsock);
	siglongjmp(timebuf, sig);
}

