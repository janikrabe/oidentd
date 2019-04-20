/*
** oidentd_util.c - oidentd utility functions.
** Copyright (c) 2001-2006 Ryan McCabe <ryan@numb.org>
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

#define _GNU_SOURCE
#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "oidentd.h"
#include "util.h"
#include "inet_util.h"
#include "missing.h"
#include "options.h"

#if HAVE_LIBUDB
#	include <udb.h>
#endif

/*
** Seed PRNG from time-of-day clock. (FIXME: Should use entropy pool on
** systems that have one. Should prefer lrand48()(3), then random()(3)
** then rand()(3) in that order.)
** Returns 0 on success, -1 on failure.
*/

int random_seed(void) {
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
	srand(0);
#else
	struct timeval tv;

	if (gettimeofday(&tv, NULL))
		return -1;

	srand((u_int32_t) (tv.tv_sec ^ tv.tv_usec));
#endif
	return 0;
}

/*
** Find the user specified by "temp_user"
** Returns non-zero on failure.
*/

int find_user(const char *temp_user, uid_t *uid) {
	struct passwd *pw;

	pw = getpwnam(temp_user);
	if (!pw) {
		char *end;
		unsigned long int temp_uid = strtoul(temp_user, &end, 10);

		if (*end != '\0')
			return -1;

		if (temp_uid >= MISSING_UID)
			return -1;

		*uid = (uid_t) temp_uid;
	} else {
		*uid = pw->pw_uid;
	}

	return 0;
}

/*
** Find the group specified by "temp_group"
** Returns non-zero on failure.
*/

int find_group(const char *temp_group, gid_t *gid) {
	struct group *gr;

	gr = getgrnam(temp_group);
	if (!gr) {
		char *end;
		unsigned long int temp_gid = strtoul(temp_group, &end, 10);

		if (*end != '\0')
			return -1;

		if (temp_gid >= MISSING_GID)
			return -1;

		*gid = (gid_t) temp_gid;
	} else {
		*gid = gr->gr_gid;
	}

	return 0;
}

/*
** Drop privileges and run with specified UID/GID.
** Returns 0 on success, -1 on failure.
*/

int drop_privs(uid_t new_uid, gid_t new_gid) {

	if (opt_enabled(CHANGE_GID)) {
		if (setgid(new_gid) != 0) {
			debug("setgid(%u): %s", new_gid, strerror(errno));
			return -1;
		}
	}

	if (opt_enabled(CHANGE_UID)) {
		struct passwd *pw;
		gid_t my_gid;
		int ret;

		pw = getpwuid(new_uid);
		if (!pw) {
			debug("getpwuid(%u): No such user", new_uid);
			return -1;
		}

		if (opt_enabled(CHANGE_GID))
			my_gid = new_gid;
		else
			my_gid = pw->pw_gid;

		ret = initgroups(pw->pw_name, my_gid);

		if (ret != 0) {
			debug("initgroups(%s, %u): %s", pw->pw_name, my_gid,
				strerror(errno));
			return -1;
		}

		if (setuid(new_uid) != 0) {
			debug("setuid(%u): %s", new_uid, strerror(errno));
			return -1;
		}
	}

	return 0;
}

/*
** Safely open "filename" which is located in the home directory
** of the user specified by "pw." This function is safe with respect
** to stat/open races. Only files owned by the user specified by "pw"
** will be opened.
**
** Returns a pointer to the FILE struct returned by fopen on success,
** NULL on failure.
*/

FILE *safe_open(const struct passwd *pw, const char *filename) {
	size_t len;
	char *path;
	struct stat st;
	FILE *fp;

	len = strlen(pw->pw_dir) + strlen(filename) + 2;

	path = xmalloc(len);
	snprintf(path, len, "%s/%s", pw->pw_dir, filename);

	if (stat(path, &st) != 0)
		goto out_fail;

	if (st.st_uid != pw->pw_uid) {
		o_log(LOG_CRIT,
			"Refused to read %s during request for %s (owner is UID %d)",
			path, pw->pw_name, st.st_uid);

		goto out_fail;
	}

	fp = fopen(path, "r");
	if (!fp) {
		if (errno != ENOENT)
			debug("open: %s: : %s", path, strerror(errno));

		goto out_fail;
	}

	if (fstat(fileno(fp), &st) != 0) {
		debug("stat: %s: %s", path, strerror(errno));

		fclose(fp);
		goto out_fail;
	}

	if (st.st_uid != pw->pw_uid) {
		o_log(LOG_CRIT,
			"Refused to read \"%s\" during request for %s (owner is UID %u)",
			path, pw->pw_name, st.st_uid);

		fclose(fp);
		goto out_fail;
	}

	free(path);
	return fp;

out_fail:
	free(path);
	return NULL;
}

/*
** Go background.
*/

int go_background(void) {
	int fd;

	switch (fork()) {
		case -1:
			return -1;
		case 0:
			break;
		default:
			_exit(EXIT_SUCCESS);
	}

	if (setsid() == (pid_t) -1) {
		debug("setsid: %s", strerror(errno));
		return -1;
	}

	if (chdir("/") != 0) {
		debug("chdir: %s", strerror(errno));
		return -1;
	}

	umask(DEFAULT_UMASK);

	fd = open("/dev/null", O_RDWR);
	if (fd == -1) {
		debug("open: /dev/null: %s", strerror(errno));
		return -1;
	}

	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	return 0;
}

/*
** Same as malloc(3), except exits on failure.
*/

void *xmalloc(size_t size) {
	void *ret = malloc(size);

	if (!ret) {
		o_log(LOG_CRIT, "Fatal: malloc: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return ret;
}

/*
** Same as calloc(3), except exits on failure.
*/

void *xcalloc(size_t nmemb, size_t size) {
	void *ret = calloc(nmemb, size);

	if (!ret) {
		o_log(LOG_CRIT, "Fatal: calloc: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return ret;
}

/*
** Same as realloc(3), except exits on failure.
*/

void *xrealloc(void *ptr, size_t len) {
	void *ret = realloc(ptr, len);

	if (!ret) {
		o_log(LOG_CRIT, "Fatal: realloc: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return ret;
}

/*
** Copy at most n-1 characters from src to dest and NULL-terminate dest.
** Returns a pointer to the destination string.
*/

char *xstrncpy(char *dest, const char *src, size_t n) {
	char *ret = dest;

	if (n == 0)
		return dest;

	while (--n > 0 && (*dest++ = *src++) != '\0')
		;

	*dest = '\0';

	return ret;
}

/*
** Same as strdup(3), except exits on failure, and returns NULL
** if passed a NULL pointer.
*/

char *xstrdup(const char *string) {
	char *ret;

	if (!string)
		return NULL;

	ret = strdup(string);

	if (!ret) {
		o_log(LOG_CRIT, "Fatal: strdup: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return ret;
}

/*
** Add a link for "new_data" to the front of the linked list, "list".
** Returns the new list.
*/

list_t *list_prepend(list_t **list, void *new_data) {
	list_t *new_entry = xmalloc(sizeof(list_t));

	new_entry->next = *list;
	new_entry->data = new_data;
	*list = new_entry;

	return *list;
}

/*
** Destroy the linked list, "list"
*/

void list_destroy(list_t *list, void (*free_data)(void *)) {
	list_t *cur = list;

	while (cur) {
		list_t *next = cur->next;

		if (free_data)
			free_data(cur->data);

		free(cur);
		cur = next;
	}
}

/*
** Logging mechanism for oidentd.
*/

int o_log(int priority, const char *fmt, ...) {
	va_list ap;
	int ret;
	char *buf;

	if (opt_enabled(QUIET) && priority != LOG_CRIT)
		return 0;

	if (priority == LOG_DEBUG && !opt_enabled(DEBUG_MSGS))
		return 0;

	va_start(ap, fmt);
	ret = vasprintf((char **) &buf, fmt, ap);
	va_end(ap);

	if (opt_enabled(NOSYSLOG) || isatty(fileno(stderr)))
		fprintf(stderr, "%s\n", buf);
	else
		syslog(priority, "%s", buf);

	free(buf);
	return ret;
}

#if HAVE_LIBUDB
/*
** Look up a connection in the UDB shared memory tables.
**
** This only supports IPv4 right now.
*/

struct udb_lookup_res get_udb_user(	in_port_t lport,
					in_port_t fport,
					const struct sockaddr_storage *laddr,
					const struct sockaddr_storage *faddr,
					int sock)
{
	struct udb_connection conn;
	struct udb_conn_user buf;
	struct udb_lookup_res res = {0, (uid_t) -1};
	struct passwd *pw;
	char faddr_buf[MAX_IPLEN];
	char laddr_buf[MAX_IPLEN];
	extern char *ret_os;

	if (laddr->ss_family != AF_INET || faddr->ss_family != AF_INET)
		return res;

	memset(&conn, 0, sizeof(conn));

	conn.from.sin_family = laddr->ss_family;
	memcpy(&conn.from.sin_addr, &SIN4(laddr)->sin_addr,
		sizeof(conn.from.sin_addr));
	conn.from.sin_port = htons(lport);

	conn.to.sin_family = faddr->ss_family;
	memcpy(&conn.to.sin_addr, &SIN4(faddr)->sin_addr, sizeof(conn.to.sin_addr));
	conn.to.sin_port = htons(fport);

	get_ip(faddr, faddr_buf, sizeof(faddr_buf));
	get_ip(laddr, laddr_buf, sizeof(laddr_buf));

	debug("UDB lookup: %s:%d->%s:%d",
		laddr_buf, ntohs(conn.from.sin_port),
		faddr_buf, ntohs(conn.to.sin_port));

	if (!udb_conn_get(&conn, &buf))
		return res;

	/* If the user is local, return their UID */
	pw = getpwnam(buf.username);
	if (pw) {
		res.status = 1;
		res.uid = pw->pw_uid;
		return res;
	}

	/* User not local, reply with string from UDB table. */
	sockprintf(sock, "%d,%d:USERID:%s:%s\r\n",
		lport, fport, ret_os, buf.username);

	o_log(LOG_INFO, "[%s] UDB lookup: %d , %d : (returned %s)",
		faddr_buf, lport, fport, buf.username);

	res.status = 2;
	return res;
}

#endif
