/*
** oidentd.h - oidentd ident (RFC 1413) implementation.
** Copyright (c) 1998-2006 Ryan McCabe <ryan@numb.org>
** Copyright (c) 2018      Janik Rabe  <oidentd@janikrabe.com>
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

#ifndef __OIDENTD_H_
#define __OIDENTD_H_

#ifndef SYSCONFDIR
	#define SYSCONFDIR "/etc"
#endif

/*
** File containing the ident replies to be used for hosts that
** masquerade through us.
*/

#define MASQ_MAP		SYSCONFDIR "/oidentd_masq.conf"

/*
** String prepended to a random number when the random_numeric capability is
** used.
*/

#define UPREFIX			"user"

/*
** System-wide configuration file.
*/

#define CONFFILE		SYSCONFDIR "/oidentd.conf"

/*
** Per-user configuration file.  This file is relative to the user's
** home directory.
*/

#if XDGBDIR_SUPPORT
#	define USER_CONF_XDG ".config/oidentd.conf"
#endif

#define USER_CONF		".oidentd.conf"

/*
** Maximum length of ident replies.
*/

#define MAX_ULEN		512

/*
** Maximum number of random replies -- the number of strings that
** may follow "reply" statements.
*/

#define MAX_RANDOM_REPLIES	20

/*
** The default UID and GID, respectively, that oidentd will
** run with.
*/

#define DEFAULT_UID		65534
#define DEFAULT_GID		65534

/*
** Default port oidentd runs on and default forward port.
**
** If you're going to change these default ports, be sure to make them a
** string (eg. "113" not 113).
*/

#define DEFAULT_PORT	"113"
#define DEFAULT_FPORT	"113"

/*
** The amount of time oidentd will wait for client input
** before it drops the connection.
*/

#define DEFAULT_TIMEOUT	30

/*
** Nothing below here should need to be changed.
*/

#define DEFAULT_UMASK	0022

#define MAX_HOSTLEN		256

#ifndef INET_ADDRSTRLEN
#	define INET_ADDRSTRLEN		16
#endif

#if WANT_IPV6
#	ifndef INET6_ADDRSTRLEN
#		define INET6_ADDRSTRLEN	46
#	endif
#	if INET6_ADDRSTRLEN < 46
#		error "INET6_ADDRSTRLEN is too small"
#	endif
#	define MAX_IPLEN	INET6_ADDRSTRLEN
#	define IP_SCAN_SPEC	"%45s"
#else
#	if INET_ADDRSTRLEN < 16
#		error "INET_ADDRSTRLEN is too small"
#	endif
#	define MAX_IPLEN	INET_ADDRSTRLEN
#	define IP_SCAN_SPEC	"%15s"
#endif

#define PORT_MAX		0xffff
#define PORT_MIN		1

/*
** POSIX reserves these UID and GID values for this purpose.
*/

#define MISSING_UID		((uid_t) -1)
#define MISSING_GID		((gid_t) -1)

#define VALID_PORT(p)	((p) >= PORT_MIN && ((p) & PORT_MAX) == (p))
#define ERROR(x)		(opt_enabled(HIDE_ERRORS) ? "UNKNOWN-ERROR" : (x))

#if ENABLE_DEBUGGING
#	define debug(format, args...) do { o_log(DEBUG, "[%s:%u:%s] DEBUG: " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args); } while (0)
#else
#	define debug(format, args...) do { } while (0)
#endif

#ifdef HAVE___ATTRIBUTE__UNUSED
	#define __notused __attribute__((unused))
#else
	#define __notused
#endif

#ifdef HAVE___ATTRIBUTE__NORETURN
	#define __noreturn __attribute__((noreturn))
#else
	#define __noreturn
#endif

#ifdef HAVE___ATTRIBUTE__FORMAT
	#define __format(x) __attribute__((format x ))
#else
	#define __format(x)
#endif

#ifdef HAVE_STDBOOL_H
#	include <stdbool.h>
#else
typedef enum {
	false = 0,
	true = 1
} bool;
#endif

int k_open(void);

#ifndef HAVE_STRUCT_SOCKADDR_STORAGE

struct sockaddr_storage {
	struct	sockaddr __ss_sockaddr;
	char	__ss_pad[128 - sizeof(struct sockaddr)];
};

#	define ss_family __ss_sockaddr.sa_family

#endif

#ifdef HAVE___SS_FAMILY
	#define ss_family __ss_family
#endif

#if LIBNFCT_SUPPORT
	bool drop_privs_libnfct(uid_t uid, gid_t gid);
#endif

/*
** System-dependent initialization; called only once.
** Called before privileges are dropped.
** Returns non-zero on failure.
*/

int core_init(void);

/*
** Returns the UID of the owner of an IPv4 connection,
** or MISSING_UID on failure.
*/

uid_t get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr);

/*
** Returns the UID of the owner of an IPv6 connection,
** or MISSING_UID on failure.
*/

uid_t get_user6(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr);

int read_config(const char *config_file);

#endif
