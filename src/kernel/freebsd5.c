/*
** freebsd5.c - Ident lookup routines for FreeBSD >= 5
** Copyright (c) 2000-2006 Ryan McCabe <ryan@numb.org>
** Copyright (c) 2018-2019 Janik Rabe  <info@janikrabe.com>
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
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/filio.h>
#include <sys/ioccom.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/ucred.h>
#include <sys/uio.h>
#include <sys/utsname.h>

#include "oidentd.h"
#include "util.h"
#include "inet_util.h"
#include "missing.h"
#include "options.h"

/*
** System-dependent initialization; called only once.
** Called before privileges are dropped.
** Returns non-zero on failure.
*/

int core_init(void) {
	return 0;
}

extern struct sockaddr_storage proxy;

/*
** Returns the UID of the owner of an IPv4 connection,
** or MISSING_UID on failure.
*/

uid_t get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct xucred xuc;
	struct sockaddr_in sin4[2];
	size_t len;
	int ret;

	len = sizeof(xuc);

	memset(sin4, 0, sizeof(sin4));

	sin4[0].sin_len = sizeof(struct sockaddr_in);
	sin4[0].sin_family = AF_INET;
	sin4[0].sin_port = lport;
	sin4[0].sin_addr.s_addr = SIN4(laddr)->sin_addr.s_addr;

	sin4[1].sin_len = sizeof(struct sockaddr_in);
	sin4[1].sin_family = AF_INET;
	sin4[1].sin_port = fport;

	if (!opt_enabled(PROXY) || !sin_equal(faddr, &proxy))
		sin4[1].sin_addr.s_addr = SIN4(faddr)->sin_addr.s_addr;

	ret = sysctlbyname("net.inet.tcp.getcred",
		&xuc, &len, sin4, sizeof(sin4));

	if (ret == -1) {
		debug("sysctlbyname: %s", strerror(errno));
		return MISSING_UID;
	}

	if (xuc.cr_version != XUCRED_VERSION) {
		debug("kernel is using xucred version %u, expected %u",
			xuc.cr_version, XUCRED_VERSION);
		return MISSING_UID;
	}

	return xuc.cr_uid;
}

#if WANT_IPV6

/*
** Returns the UID of the owner of an IPv6 connection,
** or MISSING_UID on failure.
*/

uid_t get_user6(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct xucred xuc;
	struct sockaddr_in6 sin6[2];
	size_t len;
	int ret;

	len = sizeof(xuc);

	memset(sin6, 0, sizeof(sin6));

	sin6[0].sin6_len = sizeof(struct sockaddr_in6);
	sin6[0].sin6_family = AF_INET6;
	sin6[0].sin6_port = lport;
	memcpy(&sin6[0].sin6_addr, &SIN6(laddr)->sin6_addr,
		sizeof(sin6[0].sin6_addr));

	sin6[1].sin6_len = sizeof(struct sockaddr_in6);
	sin6[1].sin6_family = AF_INET6;
	sin6[1].sin6_port = fport;
	memcpy(&sin6[1].sin6_addr, &SIN6(faddr)->sin6_addr,
		sizeof(sin6[1].sin6_addr));

	ret = sysctlbyname("net.inet6.tcp6.getcred",
		&xuc, &len, sin6, sizeof(sin6));

	if (ret == -1) {
		debug("sysctlbyname: %s", strerror(errno));
		return MISSING_UID;
	}

	if (xuc.cr_version != XUCRED_VERSION) {
		debug("kernel is using xucred version %u, expected %u",
			xuc.cr_version, XUCRED_VERSION);
		return MISSING_UID;
	}

	return xuc.cr_uid;
}

#endif

/*
** Open the kernel memory device.
** Return 0 on success, or -1 with errno set.
**
** No kmem access required; nothing to do.
*/

int k_open(void) {
	return 0;
}
