/*
** oidentd_util.h - oidentd utility functions.
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

#ifndef __OIDENTD_UTIL_H
#define __OIDENTD_UTIL_H

#define FACILITY	LOG_DAEMON
#define NORMAL		LOG_INFO
#define DEBUG		LOG_DEBUG

#ifndef MIN
#	define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

typedef struct list {
	struct list *next;
	void *data;
} list_t;

int o_log(int priority, const char *fmt, ...) __format((printf, 2, 3));
int drop_privs(uid_t new_uid, gid_t new_gid);
int go_background(void);

int get_udb_user(	in_port_t lport,
					in_port_t fport,
					const struct sockaddr_storage *laddr,
					const struct sockaddr_storage *faddr,
					int sock);

FILE *safe_open(const struct passwd *pw, const char *filename);

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t len);

char *xstrncpy(char *dest, const char *src, size_t n);
char *xstrdup(const char *string);

list_t *list_prepend(list_t **list, void *new_data);
void list_destroy(list_t *list, void (*free_data)(void *));

int find_user(const char *temp_user, uid_t *uid);
int find_group(const char *temp_group, gid_t *gid);

int random_seed(void);
inline int randval(int i);

#ifndef HAVE_SNPRINTF
	int snprintf(char *str, size_t n, char const *fmt, ...);
#endif

#ifndef HAVE_VSNPRINTF
	int vsnprintf(char *str, size_t n, char *fmt, va_list ap);
#endif

#endif
