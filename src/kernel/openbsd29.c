/*
** openbsd29.c - Low level kernel access functions for OpenBSD 2.9 and greater
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

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/tcp.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_inet_util.h>
#include <oidentd_missing.h>
#include <oidentd_options.h>

extern struct sockaddr_storage proxy;

/*
** System dependant initialization. Call only once!
** On failure, return false.
*/

bool core_init(void) {
	return (true);
}

/*
** Return the UID of the connection owner
*/

int get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct tcp_ident_mapping tir;
	struct sockaddr_in *fin, *lin;
	int mib[] = { CTL_NET, PF_INET, IPPROTO_TCP, TCPCTL_IDENT };
	int error;
	size_t i;

	memset(&tir, 0, sizeof(tir));

	tir.faddr.ss_family = AF_INET;
	tir.faddr.ss_len = sizeof(struct sockaddr);
	fin = (struct sockaddr_in *) &tir.faddr;
	fin->sin_port = fport;

	if (!opt_enabled(PROXY) || !sin_equal(faddr, &proxy))
		memcpy(&fin->sin_addr, &SIN4(faddr)->sin_addr, sizeof(struct in_addr));

	tir.laddr.ss_family = AF_INET;
	tir.laddr.ss_len = sizeof(struct sockaddr);
	lin = (struct sockaddr_in *) &tir.laddr;
	lin->sin_port = lport;
	memcpy(&lin->sin_addr, &SIN4(laddr)->sin_addr, sizeof(struct in_addr));

	i = sizeof(tir);

	error = sysctl(mib, sizeof(mib) / sizeof(int), &tir, &i, NULL, 0);

	if (error == 0 && tir.ruid != -1)
		return (tir.ruid);

	if (error == -1)
		debug("sysctl: %s", strerror(errno));

	return (-1);
}

#ifdef WANT_IPV6

int get_user6(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct tcp_ident_mapping tir;
	struct sockaddr_in6 *fin;
	struct sockaddr_in6 *lin;
	int mib[] = { CTL_NET, PF_INET, IPPROTO_TCP, TCPCTL_IDENT };
	int error;
	size_t i;

	memset(&tir, 0, sizeof(tir));

	fin = (struct sockaddr_in6 *) &tir.faddr;
	fin->sin6_family = AF_INET6;
	fin->sin6_len = sizeof(struct sockaddr_in6);

	if (faddr->ss_len > sizeof(tir.faddr))
		return (-1);

	memcpy(&fin->sin6_addr, &SIN6(faddr)->sin6_addr, sizeof(tir.faddr));
	fin->sin6_port = fport;

	lin = (struct sockaddr_in6 *) &tir.laddr;
	lin->sin6_family = AF_INET6;
	lin->sin6_len = sizeof(struct sockaddr_in6);

	if (laddr->ss_len > sizeof(tir.laddr))
		return (-1);

	memcpy(&lin->sin6_addr, &SIN6(laddr)->sin6_addr, sizeof(tir.laddr));
	lin->sin6_port = lport;

	i = sizeof(tir);
	error = sysctl(mib, sizeof(mib) / sizeof(int), &tir, &i, NULL, 0);

	if (error == 0 && tir.ruid != -1)
		return (tir.ruid);

	if (error == -1)
		debug("sysctl: %s", strerror(errno));

	return (-1);
}

#endif

/*
** Stub k_open() function.
*/

int k_open(void) {
	return (0);
}
