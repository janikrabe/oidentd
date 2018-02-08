/*
** netbsd5.c - Ident lookup routines for >= NetBSD 5
** Copyright (C) 2018 Janik Rabe <info@janikrabe.com>
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
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <sys/sysctl.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

#ifdef WANT_IPV6
#	include <netinet/ip6.h>
#endif

#ifdef MASQ_SUPPORT
#	include <netinet/ip_fil.h>
#	include <netinet/ip_nat.h>
#endif

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_inet_util.h>
#include <oidentd_missing.h>
#include <oidentd_masq.h>
#include <oidentd_options.h>

/*
** System dependant initialization. Call only once!
** On failure, return false.
*/

bool core_init(void) {
	return (true);
}

int k_open(void) {
	return (0);
}

/*
** Return the UID of the connection owner
*/

int get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	int mib[] = { CTL_NET, PF_INET, IPPROTO_TCP, TCPCTL_IDENT };
	struct sockaddr_storage ss[2];
	int uid = -1;
	size_t uidlen;
	size_t sslen;
	int error;

	memcpy(&ss[0], faddr, sizeof(ss[0]));
	memcpy(&ss[1], laddr, sizeof(ss[1]));
	((struct sockaddr_in *) &ss[0])->sin_port = fport;
	((struct sockaddr_in *) &ss[1])->sin_port = lport;
	uidlen = sizeof(uid);
	sslen = sizeof(ss);
	error = sysctl(mib, sizeof(mib) / sizeof(int), &uid, &uidlen, ss, sslen);

	if (error == 0 && uid != -1)
		return uid;

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
	int mib[] = { CTL_NET, PF_INET6, IPPROTO_TCP, TCPCTL_IDENT };
	struct sockaddr_storage ss[2];
	int uid = -1;
	size_t uidlen;
	size_t sslen;
	int error;

	memcpy(&ss[0], faddr, sizeof(ss[0]));
	memcpy(&ss[1], laddr, sizeof(ss[1]));
	((struct sockaddr_in6 *) &ss[0])->sin6_port = fport;
	((struct sockaddr_in6 *) &ss[1])->sin6_port = lport;
	uidlen = sizeof(int);
	sslen = sizeof(ss);
	error = sysctl(mib, sizeof(mib) / sizeof(int), &uid, &uidlen, ss, sslen);

	if (error == 0 && uid != -1)
		return uid;

	if (error == -1)
		debug("sysctl: %s", strerror(errno));

	return (-1);
}

#endif

#ifdef MASQ_SUPPORT

/*
** Check ident requests for NAT connection
*/

int masq(	int sock,
			in_port_t lport,
			in_port_t fport,
			struct sockaddr_storage *laddr,
			struct sockaddr_storage *faddr)
{
	// TODO Not yet implemented.
	return (-1);
}

#endif
