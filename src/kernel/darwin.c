/*
** darwin.c - Low level kernel access functions for Apple's Darwin OS.
**
** This is basically a modified version of the NetBSD file found in
** this directory.  It's distributed under the same copyright.
** The original copyright notice is as follows:
**
**		This program is in the public domain and may be used freely
**		by anyone who wants to.
**
** NAT code taken from the OpenBSD NAT code by
** Slawomir Piotrowski <slawek@telsatgp.com.pl>
**
** Modifications Copyright (c) 1998-2006 Ryan McCabe <ryan@numb.org>
** Modifications Copyright (c) 2018      Janik Rabe  <oidentd@janikrabe.com>
**
** All IPv6 code Copyright (c) 2002-2006 Ryan McCabe <ryan@numb.org>
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <kvm.h>
#include <pwd.h>
#include <nlist.h>
#include <limits.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#ifdef HAVE_NETINET_IP_COMPAT_H
#	include <netinet/ip_compat.h>
#endif

#define KERNEL
#include <sys/file.h>
#undef KERNEL

#if WANT_IPV6
#	include <netinet/tcp_timer.h>
#	include <netinet/ip6.h>
#	ifdef HAVE_NETINET6_IN6_PCB_H
#		include <netinet6/in6_pcb.h>
#	endif
#endif

#if MASQ_SUPPORT
#	include <netinet/ip_fil.h>
#	include <netinet/ip_proxy.h>
#	include <netinet/ip_nat.h>
#endif

#include "oidentd.h"
#include "util.h"
#include "inet_util.h"
#include "missing.h"
#include "masq.h"
#include "options.h"

#define N_TCB		0
#define N_TCB6		1

#if MASQ_SUPPORT
#	define N_NATLIST	2
#	define N_TOTAL		3
#else
#	define N_TOTAL		2
#endif

extern struct sockaddr_storage proxy;

static int getbuf(u_long addr, void *buf, size_t len);

static struct socket *getlist4(	struct inpcbhead *pcbhead,
								in_port_t lport,
								in_port_t fport,
								const struct in_addr *laddr,
								const struct in_addr *faddr);
static struct kainfo {
	kvm_t *kd;
	struct nlist nl[N_TOTAL];
} *kinfo;

int k_open(void) {
	kinfo = xmalloc(sizeof(struct kainfo));

	kinfo->kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
	if (!kinfo->kd) {
		free(kinfo);
		debug("kvm_open: %s", strerror(errno));
		return (-1);
	}

	kinfo->nl[N_TCB].n_name = "_tcb";

#if WANT_IPV6
	kinfo->nl[N_TCB6].n_name = "_tcb6";
#else
	kinfo->nl[N_TCB6].n_name = "_oidentd_nonexistent";
#endif

#if MASQ_SUPPORT
	if (opt_enabled(MASQ))
		kinfo->nl[N_NATLIST].n_name = "_nat_instances";
	else
		kinfo->nl[N_NATLIST].n_name = "NULL";
#endif

	kinfo->nl[N_TOTAL - 1].n_name = NULL;

	if (kvm_nlist(kinfo->kd, kinfo->nl) == -1) {
		kvm_close(kinfo->kd);
		free(kinfo);
		debug("kvm_nlist: %s", strerror(errno));
		return (-1);
	}

#if MASQ_SUPPORT
	if (opt_enabled(MASQ) && kinfo->nl[N_NATLIST].n_value == 0) {
		o_log(LOG_CRIT, "NAT/IP masquerading support is unavailable");
		disable_opt(MASQ);
	}
#endif

	return (0);
}

/*
** Get a piece of kernel memory with error handling.
** Returns 1 if call succeeded, else 0 (zero).
*/

static int getbuf(u_long addr, void *buf, size_t len) {

	if (kvm_read(kinfo->kd, addr, buf, len) < 0) {
		debug("getbuf: kvm_read(%08lx, %d): %s",
			addr, len, strerror(errno));

		return (-1);
	}

	return (0);
}

/*
** Traverse the inpcb list until a match is found.
** Returns NULL if no match.
*/

static struct socket *getlist4(	struct inpcbhead *pcbhead,
								in_port_t lport,
								in_port_t fport,
								const struct in_addr *laddr,
								const struct in_addr *faddr)
{
	struct inpcb *pcbp, pcb;

	if (!pcbhead)
		return (NULL);

	pcbp = pcbhead->lh_first;
	while (pcbp) {
		if (getbuf((u_long) pcbp, &pcb, sizeof(struct inpcb)) == -1)
			break;

		if (opt_enabled(PROXY)) {
			if (faddr->s_addr == SIN4(&proxy)->sin_addr.s_addr &&
				laddr->s_addr != SIN4(&proxy)->sin_addr.s_addr &&
				pcb.inp_fport == fport &&
				pcb.inp_lport == lport)
			{
				return (pcb.inp_socket);
			}
		}

		if (pcb.inp_faddr.s_addr == faddr->s_addr &&
			pcb.inp_laddr.s_addr == laddr->s_addr &&
			pcb.inp_fport == fport &&
			pcb.inp_lport == lport)
		{
			return (pcb.inp_socket);
		}

		pcbp = pcb.inp_list.le_next;
	}

	return (NULL);
}

/*
** System-dependent initialization; called only once.
** Called before privileges are dropped.
** Returns false on failure.
*/

bool core_init(void) {
	return (true);
}

/*
** Returns the UID of the owner of an IPv4 connection,
** or MISSING_UID on failure.
*/

uid_t get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct socket *sockp, sock;
	struct inpcbhead tcb;
	int ret;

	ret = getbuf(kinfo->nl[N_TCB].n_value, &tcb, sizeof(tcb));
	if (ret == -1)
		return (MISSING_UID);

	sockp = getlist4(&tcb, lport, fport,
				&SIN4(laddr)->sin_addr, &SIN4(faddr)->sin_addr);

	if (!sockp)
		return (MISSING_UID);

	ret = getbuf((u_long) sockp, &sock, sizeof(sock));
	if (ret == -1)
		return (MISSING_UID);

	return (sock.so_uid);
}

#if MASQ_SUPPORT

/*
** Handle a request to a host that's IP masquerading through us.
** Returns true on success, false on failure.
*/

bool masq(	int sock,
			in_port_t lport,
			in_port_t fport,
			struct sockaddr_storage *laddr,
			struct sockaddr_storage *faddr)
{
	nat_t *np;
	nat_t nat;
	char os[24];
	char user[MAX_ULEN];
	struct sockaddr_storage ss;

	/*
	** Only IPv4 is supported right now..
	*/

	if (faddr->ss_family != AF_INET || laddr->ss_family != AF_INET)
		return (false);

	if (getbuf(kinfo->nl[N_NATLIST].n_value, &np, sizeof(np)) == -1)
		return (false);

	for (; np; np = nat.nat_next) {
		int retf;
		int retm;
		in_port_t masq_lport;
		in_port_t masq_fport;

		if (getbuf((u_long) np, &nat, sizeof(nat)) == -1)
			break;

		if (nat.nat_p != IPPROTO_TCP)
			continue;

		if (lport != nat.nat_outport)
			continue;

		if (fport != nat.nat_oport)
			continue;

		if (SIN4(laddr)->sin_addr.s_addr != nat.nat_outip.s_addr)
			continue;

		if (SIN4(faddr)->sin_addr.s_addr != nat.nat_oip.s_addr) {
			if (!opt_enabled(PROXY))
				continue;

			if (SIN4(faddr)->sin_addr.s_addr != SIN4(&proxy)->sin_addr.s_addr)
				continue;

			if (SIN4(laddr)->sin_addr.s_addr == SIN4(&proxy)->sin_addr.s_addr)
				continue;
		}

		lport = ntohs(lport);
		fport = ntohs(fport);
		masq_lport = ntohs(nat.nat_inport);
		masq_fport = ntohs(nat.nat_outport);

		sin_setv4(nat.nat_inip.s_addr, &ss);

		retm = find_masq_entry(&ss, user, sizeof(user), os, sizeof(os));

		if (opt_enabled(FORWARD) && (retm != 0 || !opt_enabled(MASQ_OVERRIDE))) {
			retf = fwd_request(sock, lport, masq_lport, fport, masq_fport, &ss);
			if (retf == 0) {
				if (retm != 0)
					return (true);
			} else {
				char ipbuf[MAX_IPLEN];

				get_ip(&ss, ipbuf, sizeof(ipbuf));
				debug("Forward to %s (%d %d) failed",
					ipbuf, nat.nat_inport, fport);
			}
		}

		if (retm == 0) {
			char ipbuf[MAX_IPLEN];

			sockprintf(sock, "%d,%d:USERID:%s:%s\r\n",
				lport, fport, os, user);

			get_ip(faddr, ipbuf, sizeof(ipbuf));

			o_log(NORMAL,
				"[%s] (NAT) Successful lookup: %d , %d : %s",
				ipbuf, lport, fport, user);

			return (true);
		}
	}

	return (false);
}

#endif

#if WANT_IPV6

/*
** Traverse the tcb6 list until a match is found.
** Returns NULL if no match.
*/

static struct socket *getlist6(	struct inpcbhead *pcbhead,
								in_port_t lport,
								in_port_t fport,
								const struct in6_addr *laddr,
								const struct in6_addr *faddr)
{
	struct in6pcb *pcb6p, pcb6;

	if (!pcbhead)
		return (NULL);

	pcb6p = pcbhead->lh_first;
	while (pcb6p) {
		if (getbuf((u_long) pcb6p, &pcb6, sizeof(struct in6pcb)) == -1)
			break;

		if (pcb6.in6p_fport == fport &&
			pcb6.in6p_lport == lport &&
			IN6_ARE_ADDR_EQUAL(&pcb6.in6p_laddr, laddr) &&
			IN6_ARE_ADDR_EQUAL(&pcb6.in6p_faddr, faddr))
		{
			return (pcb6.in6p_socket);
		}

		pcb6p = pcb6.inp_list.le_next;
	}

	return (NULL);
}

/*
** Returns the UID of the owner of an IPv6 connection,
** or MISSING_UID on failure.
*/

uid_t get_user6(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct socket *sockp, sock;
	struct inpcbhead pcb6;
	int ret;

	ret = getbuf(kinfo->nl[N_TCB6].n_value, &pcb6, sizeof(pcb6));
	if (ret == -1)
		return (MISSING_UID);

	sockp = getlist6(&pcb6, lport, fport,
				&SIN6(laddr)->sin6_addr, &SIN6(faddr)->sin6_addr);

	if (!sockp)
		return (MISSING_UID);

	ret = getbuf((u_long) sockp, &sock, sizeof(sock));
	if (ret == -1)
		return (MISSING_UID);

	return (sock.so_uid);
}

#endif
