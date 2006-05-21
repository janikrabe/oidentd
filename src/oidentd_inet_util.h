/*
** oidentd_inet_util.h - oidentd network utility functions.
** Copyright (C) 2001-2006 Ryan McCabe <ryan@numb.org>
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

#ifndef __OIDENTD_INET_UTIL_H
#define __OIDENTD_INET_UTIL_H

#define SIN4(x) ((struct sockaddr_in *) (x))
#define SIN6(x) ((struct sockaddr_in6 *) (x))

int *setup_listen(struct sockaddr_storage *listen_addr, in_port_t listen_port);

int get_port(const char *name, in_port_t *port);
int get_addr(const char *const hostname, struct sockaddr_storage *g_addr);
void get_ip(struct sockaddr_storage *ss, char *buf, size_t len);
int get_hostname(struct sockaddr_storage *addr, char *hostname, size_t len);

int sockprintf(int fd, const char *fmt, ...) __format((printf, 2, 3));
ssize_t sock_read(int fd, char *srbuf, size_t len);
ssize_t sock_write(int sock, void *buf, size_t len);

#ifndef HAVE_INET_ATON
	int inet_aton(const char *cp, struct in_addr *addr);
#endif

#ifndef HAVE_INET_NTOP
	const char *inet_ntop(int af, const void *src, char *dst, size_t len);
#endif

#ifdef WANT_IPV6
	void sin_setv6(struct in6_addr *sin6, struct sockaddr_storage *ss);
#endif

void sin_setv4(in_addr_t addr, struct sockaddr_storage *ss);
void sin_extractv4(void *sin6, struct in_addr *sin4);
size_t sin_len(const struct sockaddr_storage *ss);
size_t sin_addr_len(const struct sockaddr_storage *ss);
void sin_copy(struct sockaddr_storage *ss1, const struct sockaddr_storage *ss2);
void *sin_addr(struct sockaddr_storage *ss);
in_port_t sin_port(const struct sockaddr_storage *ss);
void sin_set_port(in_port_t port, struct sockaddr_storage *ss);
bool sin4_equal(struct sockaddr_storage *ss1, struct sockaddr_storage *ss2);
bool sin6_equal(struct sockaddr_storage *ss1, struct sockaddr_storage *ss2);
bool sin_equal(struct sockaddr_storage *ss1, struct sockaddr_storage *ss2);

#endif
