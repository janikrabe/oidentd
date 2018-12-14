/*
** linux.c - Linux user lookup facility.
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

#define _GNU_SOURCE

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netlink.h>

#include "oidentd.h"
#include "util.h"
#include "user_db.h"
#include "inet_util.h"
#include "missing.h"
#include "masq.h"
#include "options.h"
#include "netlink.h"

#if HAVE_LIBCAP_NG
#	include <cap-ng.h>
#else
#	undef LIBNFCT_SUPPORT
#endif

#if !MASQ_SUPPORT
#	undef LIBNFCT_SUPPORT
#endif

#if LIBNFCT_SUPPORT
#	include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#endif

#if HAVE_LIBUDB
#	include <udb.h>
#endif

#define CFILE		"/proc/net/tcp"
#define CFILE6		"/proc/net/tcp6"
#define MASQFILE	"/proc/net/ip_masquerade"
#define IPCONNTRACK	"/proc/net/ip_conntrack"
#define NFCONNTRACK	"/proc/net/nf_conntrack"

static int netlink_sock;
extern struct sockaddr_storage proxy;
extern char *ret_os;

#if LIBNFCT_SUPPORT
struct ct_masq_query {
	int sock;
	in_port_t lport;
	in_port_t fport;
	struct sockaddr_storage *laddr;
	struct sockaddr_storage *faddr;
	int status;
};
#endif

#if MASQ_SUPPORT
static int masq_ct_line(char *line,
			int sock,
			int ct_type,
			in_port_t lport,
			in_port_t fport,
			struct sockaddr_storage *laddr,
			struct sockaddr_storage *faddr);
#endif

#if LIBNFCT_SUPPORT
static bool dispatch_libnfct_query(struct ct_masq_query *queryp);
static int callback_nfct(enum nf_conntrack_msg_type type,
			struct nf_conntrack *ct,
			void *data);
#endif

static uid_t lookup_tcp_diag(	struct sockaddr_storage *src_addr,
							struct sockaddr_storage *dst_addr,
							in_port_t src_port,
							in_port_t dst_port);

#if MASQ_SUPPORT
enum {
	CT_UNKNOWN,
	CT_MASQFILE,
	CT_IPCONNTRACK,
	CT_NFCONNTRACK,
};
FILE *masq_fp;
static int conntrack = CT_UNKNOWN;
#endif

#if LIBNFCT_SUPPORT
bool drop_privs_libnfct(uid_t uid, gid_t gid) {
	/* drop privileges, keeping only CAP_NET_ADMIN for libnfct queries */

	int ret;
	capng_clear(CAPNG_SELECT_BOTH);
	ret = capng_update(CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED,
			CAP_NET_ADMIN);
	if (ret) {
		debug("capng_update: error %d", ret);
		return (false);
	}

	ret = capng_change_id(
			opt_enabled(CHANGE_UID) ? uid : MISSING_UID,
			opt_enabled(CHANGE_GID) ? gid : MISSING_GID,
			CAPNG_CLEAR_BOUNDING | CAPNG_DROP_SUPP_GRP);
	if (ret) {
		debug("capng_change_id: error %d", ret);
		return (false);
	}

	/* don't try to drop privileges again */
	disable_opt(CHANGE_UID);
	disable_opt(CHANGE_GID);

	return (true);
}

static bool dispatch_libnfct_query(struct ct_masq_query *queryp) {
	struct nfct_handle *nfcthp = nfct_open(CONNTRACK, 0);

	if (!nfcthp) {
		debug("nfct_open: %s", strerror(errno));
		return (false);
	}

	if (nfct_callback_register(nfcthp, NFCT_T_ALL,
			callback_nfct, (void *) queryp)) {
		debug("nfct_callback_register: %s", strerror(errno));
		return (false);
	}

	if (nfct_query(nfcthp, NFCT_Q_DUMP, &queryp->faddr->ss_family)) {
		debug("nfct_query: %s", strerror(errno));
		return (false);
	}

	if (nfct_close(nfcthp)) {
		debug("nfct_close: %s", strerror(errno));
		return (false);
	}

	return (!queryp->status);
}

/*
** Callback for libnetfilter_conntrack queries
*/

static int callback_nfct(enum nf_conntrack_msg_type type __notused,
			struct nf_conntrack *ct,
			void *data) {
	char buf[1024];
	struct ct_masq_query *query;
	int ret;

	nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN,
			NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);

	query = (struct ct_masq_query *) data;
	ret = masq_ct_line(buf, query->sock, CT_NFCONNTRACK,
			query->lport, query->fport,
			query->laddr, query->faddr);

	if (ret == 1)
		return (NFCT_CB_CONTINUE);

	query->status = ret;
	return (NFCT_CB_STOP);
}
#endif

/*
** System-dependent initialization; called only once.
** Called before privileges are dropped.
** Returns non-zero on failure.
*/

int core_init(void) {
#if MASQ_SUPPORT
	if (!opt_enabled(MASQ)) {
		masq_fp = NULL;
		return (0);
	}

	masq_fp = fopen(MASQFILE, "r");
	if (!masq_fp) {
		if (errno != ENOENT) {
			o_log(LOG_CRIT, "fopen: %s: %s", MASQFILE, strerror(errno));
			return (-1);
		}

		masq_fp = fopen(NFCONNTRACK, "r");
		if (!masq_fp) {
			if (errno != ENOENT) {
				o_log(LOG_CRIT, "fopen: %s: %s", NFCONNTRACK, strerror(errno));
				return (-1);
			}

			masq_fp = fopen(IPCONNTRACK, "r");
			if (!masq_fp) {
				if (errno != ENOENT) {
					o_log(LOG_CRIT, "fopen: %s: %s", IPCONNTRACK, strerror(errno));
					return (-1);
				}

#	if LIBNFCT_SUPPORT
				return (0);
#	else
				o_log(LOG_CRIT, "NAT/IP masquerading support is unavailable "
				                "because " PACKAGE_NAME " was compiled without "
				                "libnetfilter_conntrack support and no "
				                "connection tracking file was found.");
				disable_opt(MASQ);
#	endif
			} else {
				conntrack = CT_IPCONNTRACK;
			}
		} else {
			conntrack = CT_NFCONNTRACK;
		}
	} else if (opt_enabled(PROXY) || opt_enabled(FORWARD)) {
		o_log(LOG_CRIT, "Only local NAT is supported on your system; "
		                "please consider upgrading your kernel");
		return (-1);
	} else {
		conntrack = CT_MASQFILE;
	}
#endif
	return (0);
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
	FILE *fp;
	char buf[1024];

	if (netlink_sock != -1) {
		uid_t uid = lookup_tcp_diag(laddr, faddr, lport, fport);

		if (uid != MISSING_UID)
			return (uid);
	}

	lport = ntohs(lport);
	fport = ntohs(fport);

	fp = fopen(CFILE6, "r");
	if (!fp) {
		debug("fopen: %s: %s", CFILE6, strerror(errno));
		return (MISSING_UID);
	}

	/* Eat the header line. */
	if (!fgets(buf, sizeof(buf), fp)) {
		debug("fgets: %s: Could not read header", CFILE6);
		fclose(fp);
		return (MISSING_UID);
	}

	while (fgets(buf, sizeof(buf), fp)) {
		struct in6_addr remote6;
		struct in6_addr local6;
		u_int32_t portl_temp;
		u_int32_t portf_temp;
		in_port_t portl;
		in_port_t portf;
		uid_t uid;
		int ret;
		u_long inode;

		ret = sscanf(buf,
			"%*d: %8x%8x%8x%8x:%x %8x%8x%8x%8x:%x %*x %*X:%*X %*x:%*X %*x %u %*d %lu",
			&local6.s6_addr32[0], &local6.s6_addr32[1], &local6.s6_addr32[2],
			&local6.s6_addr32[3], &portl_temp,
			&remote6.s6_addr32[0], &remote6.s6_addr32[1], &remote6.s6_addr32[2],
			&remote6.s6_addr32[3], &portf_temp,
			&uid, &inode);

		if (ret != 12)
			continue;

		portl = (in_port_t) portl_temp;
		portf = (in_port_t) portf_temp;

		if (!memcmp(&local6, sin_addr(laddr), sizeof(local6)) &&
			!memcmp(&remote6, sin_addr(faddr), sizeof(remote6)) &&
			portl == lport &&
			portf == fport)
		{
			fclose(fp);

			if (inode == 0 && uid == 0)
				return (MISSING_UID);

			return (uid);
		}
	}

	fclose(fp);
	return (MISSING_UID);
}

#endif

/*
** Returns the UID of the owner of an IPv4 connection,
** or MISSING_UID on failure.
*/

uid_t get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	uid_t uid;
	FILE *fp;
	char buf[1024];
	u_int32_t inode;
	in_addr_t laddr4;
	in_addr_t faddr4;

	if (netlink_sock != -1) {
		uid = lookup_tcp_diag(laddr, faddr, lport, fport);

		if (uid != MISSING_UID)
			return (uid);
	}

	laddr4 = SIN4(laddr)->sin_addr.s_addr;
	faddr4 = SIN4(faddr)->sin_addr.s_addr;

	lport = ntohs(lport);
	fport = ntohs(fport);

	fp = fopen(CFILE, "r");
	if (!fp) {
		debug("fopen: %s: %s", CFILE, strerror(errno));
		return (MISSING_UID);
	}

	/* Eat the header line. */
	if (!fgets(buf, sizeof(buf), fp)) {
		debug("fgets: %s: Could not read header", CFILE);
		fclose(fp);
		return (MISSING_UID);
	}

	/*
	** The line should never be longer than 1024 chars, so fgets should be OK.
	*/

	while (fgets(buf, sizeof(buf), fp)) {
		int ret;
		u_int32_t portl_temp;
		u_int32_t portf_temp;
		in_port_t portl;
		in_port_t portf;
		in_addr_t local;
		in_addr_t remote;

		ret = sscanf(buf,
			"%*d: %x:%x %x:%x %*x %*x:%*x %*x:%*x %*x %u %*d %u",
			&local, &portl_temp, &remote, &portf_temp, &uid, &inode);

		if (ret != 6)
			continue;

		portl = (in_port_t) portl_temp;
		portf = (in_port_t) portf_temp;

		if (opt_enabled(PROXY)) {
			if (faddr4 == SIN4(&proxy)->sin_addr.s_addr &&
				remote != SIN4(&proxy)->sin_addr.s_addr &&
				lport == portl &&
				fport == portf)
			{
				goto out_success;
			}
		}

		if (local == laddr4 &&
			remote == faddr4 &&
			portl == lport &&
			portf == fport)
		{
			goto out_success;
		}
	}

	fclose(fp);
	return (MISSING_UID);

out_success:
	fclose(fp);

	/*
	** If the inode is zero, the socket is dead, and its owner
	** has probably been set to root.  It would be incorrect
	** to return a successful response here.
	*/

	if (inode == 0 && uid == 0)
		return (MISSING_UID);

	return (uid);
}

#if MASQ_SUPPORT

/*
** Handle a request to a host that's IP masquerading through us.
** Returns non-zero on failure.
*/

int masq(	int sock,
			in_port_t lport,
			in_port_t fport,
			struct sockaddr_storage *laddr,
			struct sockaddr_storage *faddr)
{
	char buf[1024];
#if LIBNFCT_SUPPORT
	struct ct_masq_query query;
#endif

	lport = ntohs(lport);
	fport = ntohs(fport);

#if LIBNFCT_SUPPORT
	query = (struct ct_masq_query) { sock, lport, fport, laddr, faddr, 1 };

	if (dispatch_libnfct_query(&query))
		return (0);
#endif

	if (masq_fp) {
		/* rewind fp to read new contents */
		rewind(masq_fp);

		if (conntrack == CT_MASQFILE) {
			/* eat the header line */
			if (!fgets(buf, sizeof(buf), masq_fp)) {
				debug("fgets: conntrack file: Could not read header");
				return (-1);
			}
		}

		while (fgets(buf, sizeof(buf), masq_fp)) {
			int ret = masq_ct_line(buf, sock, conntrack,
				lport, fport, laddr, faddr);
			if (ret == 1)
				continue;
			return (ret);
		}
	} else if (conntrack != CT_UNKNOWN)
		debug("Connection tracking file is in use but not open");

	return (-1);
}

/*
** Process a connection tracking file entry.
** The lport and fport arguments are in host byte order.
** Returns -1 if an error occurred.
** Returns  0 if the entry matched and the request has been handled.
** Returns  1 if the entry did not match the query.
**/

static int masq_ct_line(char *line,
			int sock,
			int ct_type,
			in_port_t lport,
			in_port_t fport,
			struct sockaddr_storage *laddr,
			struct sockaddr_storage *faddr) {
	char os[24];
	char family[16];
	char proto[16];
	in_port_t mport;
	in_port_t nport;
	in_port_t masq_lport;
	in_port_t masq_fport;
	char user[MAX_ULEN];
	struct sockaddr_storage localm_ss;
	struct sockaddr_storage remotem_ss;
	struct sockaddr_storage localn_ss;
	struct sockaddr_storage remoten_ss;
	int ret;

	if (ct_type == CT_MASQFILE) {
		in_addr_t localm4;
		in_addr_t remotem4;
		u_int32_t mport_temp;
		u_int32_t nport_temp;
		u_int32_t masq_lport_temp;
		u_int32_t masq_fport_temp;

		if (faddr->ss_family != AF_INET)
			return (-1);

		ret = sscanf(line, "%15s %X:%X %X:%X %X %X %*d %*d %*u",
				proto, &localm4, &masq_lport_temp,
				&remotem4, &masq_fport_temp, &mport_temp, &nport_temp);

		if (ret != 7)
			return (1);

		mport = (in_port_t) mport_temp;
		nport = (in_port_t) nport_temp;
		masq_lport = (in_port_t) masq_lport_temp;
		masq_fport = (in_port_t) masq_fport_temp;

		sin_setv4(localm4, &localm_ss);
		sin_setv4(remotem4, &remotem_ss);

		/* Assume local NAT. */
		sin_setv4(localm4, &remoten_ss);
		sin_setv4(remotem4, &localn_ss);
	} else if (ct_type == CT_IPCONNTRACK) {
		unsigned int ml1, ml2, ml3, ml4, mr1, mr2, mr3, mr4;
		unsigned int nl1, nl2, nl3, nl4, nr1, nr2, nr3, nr4;
		in_addr_t localm4;
		in_addr_t remotem4;
		in_addr_t localn4;
		in_addr_t remoten4;
		u_int32_t nport_temp;
		u_int32_t mport_temp;
		u_int32_t masq_lport_temp;
		u_int32_t masq_fport_temp;

		if (faddr->ss_family != AF_INET)
			return (-1);

		ret = sscanf(line,
			"%15s %*d %*d ESTABLISHED"
			" src=%u.%u.%u.%u dst=%u.%u.%u.%u sport=%u dport=%u"
			" src=%u.%u.%u.%u dst=%u.%u.%u.%u sport=%u dport=%u",
			proto,
			&ml1, &ml2, &ml3, &ml4, &mr1, &mr2, &mr3, &mr4,
			&masq_lport_temp, &masq_fport_temp,
			&nl1, &nl2, &nl3, &nl4, &nr1, &nr2, &nr3, &nr4,
			&nport_temp, &mport_temp);

		if (ret != 21) {
			ret = sscanf(line,
				"%15s %*d %*d ESTABLISHED"
				" src=%u.%u.%u.%u dst=%u.%u.%u.%u sport=%u dport=%u"
				" packets=%*d bytes=%*d"
				" src=%u.%u.%u.%u dst=%u.%u.%u.%u sport=%u dport=%u",
			proto,
			&ml1, &ml2, &ml3, &ml4, &mr1, &mr2, &mr3, &mr4,
			&masq_lport_temp, &masq_fport_temp,
			&nl1, &nl2, &nl3, &nl4, &nr1, &nr2, &nr3, &nr4,
			&nport_temp, &mport_temp);
		}

		if (ret != 21)
			return (1);

		masq_lport = (in_port_t) masq_lport_temp;
		masq_fport = (in_port_t) masq_fport_temp;

		nport = (in_port_t) nport_temp;
		mport = (in_port_t) mport_temp;

		localm4 = ml1 << 24 | ml2 << 16 | ml3 << 8 | ml4;
		remotem4 = mr1 << 24 | mr2 << 16 | mr3 << 8 | mr4;

		localn4 = nl1 << 24 | nl2 << 16 | nl3 << 8 | nl4;
		remoten4 = nr1 << 24 | nr2 << 16 | nr3 << 8 | nr4;

		sin_setv4(localm4, &localm_ss);
		sin_setv4(remotem4, &remotem_ss);
		sin_setv4(localn4, &localn_ss);
		sin_setv4(remoten4, &remoten_ss);
	} else if (ct_type == CT_NFCONNTRACK) {
		char ml[MAX_IPLEN];
		char mr[MAX_IPLEN];
		char nl[MAX_IPLEN];
		char nr[MAX_IPLEN];
		in_addr_t localm4;
		in_addr_t remotem4;
		in_addr_t localn4;
		in_addr_t remoten4;
		struct in6_addr localm6;
		struct in6_addr remotem6;
		struct in6_addr localn6;
		struct in6_addr remoten6;
		u_int32_t nport_temp;
		u_int32_t mport_temp;
		u_int32_t masq_lport_temp;
		u_int32_t masq_fport_temp;

		ret = sscanf(line,
			"%15s %*d %15s %*d %*d ESTABLISHED"
			" src=" IP_SCAN_SPEC " dst=" IP_SCAN_SPEC " sport=%d dport=%d"
			" packets=%*d bytes=%*d"
			" src=" IP_SCAN_SPEC " dst=" IP_SCAN_SPEC " sport=%d dport=%d",
			family, proto,
			ml, mr, &masq_lport_temp, &masq_fport_temp,
			nl, nr, &nport_temp, &mport_temp);

		/* Added to handle /proc/sys/net/netfilter/nf_conntrack_acct = 0 */
		if (ret != 10) {
			ret = sscanf(line,
				"%15s %*d %15s %*d %*d ESTABLISHED"
				" src=" IP_SCAN_SPEC " dst=" IP_SCAN_SPEC " sport=%d dport=%d"
				" src=" IP_SCAN_SPEC " dst=" IP_SCAN_SPEC " sport=%d dport=%d",
				family, proto,
				ml, mr, &masq_lport_temp, &masq_fport_temp,
				nl, nr, &nport_temp, &mport_temp);
		}

		if (ret != 10)
			return (1);

		switch (faddr->ss_family) {
		case AF_INET:
			if (strcasecmp(family, "ipv4"))
				return (1);

			if (inet_pton(AF_INET, ml, &localm4)  < 0 ||
			    inet_pton(AF_INET, mr, &remotem4) < 0 ||
			    inet_pton(AF_INET, nl, &localn4)  < 0 ||
			    inet_pton(AF_INET, nr, &remoten4) < 0)
				return (1);

			sin_setv4(localm4, &localm_ss);
			sin_setv4(remotem4, &remotem_ss);
			sin_setv4(localn4, &localn_ss);
			sin_setv4(remoten4, &remoten_ss);

			break;
		case AF_INET6:
			if (strcasecmp(family, "ipv6"))
				return (1);

			if (inet_pton(AF_INET6, ml, &localm6)  < 0 ||
			    inet_pton(AF_INET6, mr, &remotem6) < 0 ||
			    inet_pton(AF_INET6, nl, &localn6)  < 0 ||
			    inet_pton(AF_INET6, nr, &remoten6) < 0)
				return (1);

			sin_setv6(&localm6, &localm_ss);
			sin_setv6(&remotem6, &remotem_ss);
			sin_setv6(&localn6, &localn_ss);
			sin_setv6(&remoten6, &remoten_ss);

			break;
		default:
			debug("masq_ct_line: bad address family %d", faddr->ss_family);
			return (-1);
		}

		masq_lport = (in_port_t) masq_lport_temp;
		masq_fport = (in_port_t) masq_fport_temp;

		nport = (in_port_t) nport_temp;
		mport = (in_port_t) mport_temp;
	} else
		return (-1);

	if (strcasecmp(proto, "tcp"))
		return (1);

	if (mport != lport)
		return (1);

	if (nport != fport)
		return (1);

	/* Local NAT, don't forward or do masquerade entry lookup. */
	if (sin_equal(&localm_ss, &remoten_ss)) {
		uid_t con_uid = MISSING_UID;
		struct passwd *pw;
		char suser[MAX_ULEN];
		char ipbuf[MAX_IPLEN];

		get_ip(faddr, ipbuf, sizeof(ipbuf));

		if (con_uid == MISSING_UID && faddr->ss_family == AF_INET)
			con_uid = get_user4(htons(masq_lport), htons(masq_fport), laddr, &remotem_ss);

		if (con_uid == MISSING_UID && faddr->ss_family == AF_INET6)
			con_uid = get_user6(htons(masq_lport), htons(masq_fport), laddr, &remotem_ss);

		if (con_uid == MISSING_UID)
			return (-1);

		pw = getpwuid(con_uid);
		if (!pw) {
			sockprintf(sock, "%d,%d:ERROR:%s\r\n",
				lport, fport, ERROR("NO-USER"));

			debug("getpwuid(%u): %s", con_uid, strerror(errno));
			return (0);
		}

		ret = get_ident(pw, masq_lport, masq_fport, laddr, &remotem_ss, suser, sizeof(suser));
		if (ret == -1) {
			sockprintf(sock, "%d,%d:ERROR:%s\r\n",
				lport, fport, ERROR("HIDDEN-USER"));

			o_log(NORMAL, "[%s] %d (%d) , %d (%d) : HIDDEN-USER (%s)",
				ipbuf, lport, masq_lport, fport, masq_fport, pw->pw_name);

			return (0);
		}

		sockprintf(sock, "%d,%d:USERID:%s:%s\r\n",
			lport, fport, ret_os, suser);

		o_log(NORMAL, "[%s] Successful lookup: %d (%d) , %d (%d) : %s (%s)",
			ipbuf, lport, masq_lport, fport, masq_fport, pw->pw_name, suser);

		return (0);
	}

	if (!sin_equal(&localn_ss, faddr)) {
		if (!opt_enabled(PROXY))
			return (1);

		if (!sin_equal(faddr, &proxy))
			return (1);

		if (sin_equal(&localn_ss, &proxy))
			return (1);
	}

	ret = find_masq_entry(&localm_ss, user, sizeof(user), os, sizeof(os));

	if (opt_enabled(FORWARD) && (ret != 0 || !opt_enabled(MASQ_OVERRIDE))) {
		char ipbuf[MAX_IPLEN];

		if (fwd_request(sock, lport, masq_lport, fport, masq_fport, &localm_ss) == 0)
			return (0);

		get_ip(&localm_ss, ipbuf, sizeof(ipbuf));

		debug("Forward to %s (%d %d) failed", ipbuf, masq_lport, fport);
	}

	if (ret == 0) {
		char ipbuf[MAX_IPLEN];

		sockprintf(sock, "%d,%d:USERID:%s:%s\r\n",
			lport, fport, os, user);

		get_ip(faddr, ipbuf, sizeof(ipbuf));

		o_log(NORMAL,
			"[%s] (Masqueraded) Successful lookup: %d , %d : %s",
			ipbuf, lport, fport, user);

		return (0);
	}

	return (-1);
}

#endif

/*
** Much of the code for this function has been borrowed from
** a patch to pidentd written by Alexey Kuznetsov <kuznet@ms2.inr.ac.ru>
** and distributed with the iproute2 package.
**
** Ryan McCabe <ryan@numb.org> has made some cleanups and converted the
** routine to support both IPv4 and IPv6 queries.
*/

static uid_t lookup_tcp_diag(	struct sockaddr_storage *src_addr,
							struct sockaddr_storage *dst_addr,
							in_port_t src_port,
							in_port_t dst_port)
{
	struct sockaddr_nl nladdr;
	struct {
		struct nlmsghdr nlh;
		struct tcpdiagreq r;
	} req;
	size_t addr_len = sin_addr_len(dst_addr);
	struct iovec iov[1];
	struct msghdr msghdr;
	char buf[8192];

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = TCPDIAG_GETSOCK;
	req.nlh.nlmsg_flags = NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = 1;

	memset(&req.r, 0, sizeof(req.r));

	req.r.tcpdiag_states = ~0U;
	req.r.tcpdiag_family = dst_addr->ss_family;
	memcpy(&req.r.id.tcpdiag_dst, sin_addr(dst_addr), addr_len);
	memcpy(&req.r.id.tcpdiag_src, sin_addr(src_addr), addr_len);
	req.r.id.tcpdiag_dport = dst_port;
	req.r.id.tcpdiag_sport = src_port;
	req.r.id.tcpdiag_cookie[0] = TCPDIAG_NOCOOKIE;
	req.r.id.tcpdiag_cookie[1] = TCPDIAG_NOCOOKIE;

	iov[0].iov_base = &req;
	iov[0].iov_len = sizeof(req);

	msghdr.msg_name = &nladdr;
	msghdr.msg_namelen = sizeof(nladdr);
	msghdr.msg_iov = iov;
	msghdr.msg_iovlen = 1;
	msghdr.msg_control = NULL;
	msghdr.msg_controllen = 0;
	msghdr.msg_flags = 0;

	if (sendmsg(netlink_sock, &msghdr, 0) < 0) {
		if (errno == ECONNREFUSED) {
			close(netlink_sock);
			netlink_sock = -1;
		}

		return (MISSING_UID);
	}

	iov[0].iov_base = buf;
	iov[0].iov_len = sizeof(buf);

	while (1) {
		ssize_t ret;
		size_t uret;
		struct nlmsghdr *h;

		msghdr.msg_name = &nladdr;
		msghdr.msg_namelen = sizeof(nladdr);
		msghdr.msg_iov = iov;
		msghdr.msg_iovlen = 1;
		msghdr.msg_control = NULL;
		msghdr.msg_controllen = 0;
		msghdr.msg_flags = 0;

		ret = recvmsg(netlink_sock, &msghdr, 0);
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;

			return (MISSING_UID);
		}

		if (ret == 0)
			return (MISSING_UID);

		h = (struct nlmsghdr *) buf;

		uret = (size_t) ret;
		while (NLMSG_OK(h, uret)) {
			struct tcpdiagmsg *r;

			if (h->nlmsg_seq != 1) {
				h = NLMSG_NEXT(h, uret);
				continue;
			}

			if (h->nlmsg_type == NLMSG_DONE || h->nlmsg_type == NLMSG_ERROR)
				return (MISSING_UID);

			r = NLMSG_DATA(h);

			if (r->id.tcpdiag_dport == dst_port &&
				r->id.tcpdiag_sport == src_port &&
				!memcmp(r->id.tcpdiag_dst, sin_addr(dst_addr), addr_len) &&
				!memcmp(r->id.tcpdiag_src, sin_addr(src_addr), addr_len))
			{
				if (r->tcpdiag_inode == 0 && r->tcpdiag_uid == 0)
					return (MISSING_UID);

				return (r->tcpdiag_uid);
			}

			return (MISSING_UID);
		}

		if ((msghdr.msg_flags & MSG_TRUNC) || uret != 0)
			return (MISSING_UID);
	}

	return (MISSING_UID);
}

/*
** Just open a netlink socket here.
*/

int k_open(void) {
	netlink_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_TCPDIAG);
	return (0);
}
