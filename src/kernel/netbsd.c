/*
** netbsd.c - Low level kernel access functions for NetBSD.
**
** This file was originally taken from the pidentd 2.x software package.
** The original copyright notice is as follows:
**
**		This program is in the public domain and may be used freely
**		by anyone who wants to.
**
** NAT code taken from the OpenBSD NAT code by
** Slawomir Piotrowski <slawek@telsatgp.com.pl>
**
** Modifications Copyright (C) 1998-2006 Ryan McCabe <ryan@numb.org>
**
** All IPv6 code Copyright 2002-2006 Ryan McCabe <ryan@numb.org>
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
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip_compat.h>

#if __NetBSD_Version__ >= 399000300	/* 3.99.3 */
#define	SO_UIDINFO	/* "struct socket" contains so_uidinfo" */

#include <sys/proc.h>
#include <sys/resource.h>
#define	_KERNEL	42
#include <sys/resourcevar.h>
#undef _KERNEL
#endif

#ifdef WANT_IPV6
#	include <sys/sysctl.h>
#	include <netinet/ip_var.h>
#	include <netinet/tcp_timer.h>
#	include <netinet/tcp_var.h>
#	include <netinet/ip6.h>
#	include <netinet6/in6_pcb.h>
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

extern struct sockaddr_storage proxy;

#define N_TCB			0
#define N_TCB6			1

#ifdef MASQ_SUPPORT
#	define N_NATLIST	2
#	define N_TOTAL		4
#else
#	define N_TOTAL		3
#endif

static int getbuf(u_long addr, void *buf, size_t len);

static struct socket *getlist4(	struct inpcbtable *tcbtablep,
								struct inpcbtable *ktcbtablep,
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
	if (kinfo->kd == NULL) {
		free(kinfo);
		debug("kvm_open: %s", strerror(errno));
		return (-1);
	}

	kinfo->nl[N_TCB].n_name = "_tcbtable";

#ifdef WANT_IPV6
#if __NetBSD_Version__ >= 106250000	/* 1.6Y */
	kinfo->nl[N_TCB6].n_name = "_tcbtable";
#else
	kinfo->nl[N_TCB6].n_name = "_tcb6";
#endif
#else
	kinfo->nl[N_TCB6].n_name = "_oidentd_nonexistent";
#endif

#ifdef MASQ_SUPPORT
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

#ifdef MASQ_SUPPORT
	if (kinfo->nl[N_NATLIST].n_value == 0)
		disable_opt(MASQ);
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

static struct socket *getlist4(	struct inpcbtable *tcbtablep,
								struct inpcbtable *ktcbtablep,
								in_port_t lport,
								in_port_t fport,
								const struct in_addr *laddr,
								const struct in_addr *faddr)
{
	struct inpcb *kpcbp, pcb;

	if (tcbtablep == NULL)
		return (NULL);

	kpcbp = (struct inpcb *) tcbtablep->inpt_queue.cqh_first;
	while (kpcbp != (struct inpcb *) ktcbtablep) {
		if (getbuf((u_long) kpcbp, &pcb, sizeof(struct inpcb)) == -1)
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

		kpcbp = (struct inpcb *) pcb.inp_queue.cqe_next;
	}

	return (NULL);
}

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
	struct socket *sockp, sock;
	struct inpcbtable tcbtable;
	int ret;
#ifdef SO_UIDINFO
	struct uidinfo uidinfo;
#endif

	ret = getbuf(kinfo->nl[N_TCB].n_value, &tcbtable, sizeof(tcbtable));
	if (ret == -1)
		return (-1);

	sockp = getlist4(&tcbtable,
				(struct inpcbtable *) kinfo->nl[N_TCB].n_value,
				lport, fport, &SIN4(laddr)->sin_addr, &SIN4(faddr)->sin_addr);

	if (sockp == NULL)
		return (-1);

	if (getbuf((u_long) sockp, &sock, sizeof(sock)) == -1)
		return (-1);

#ifdef SO_UIDINFO
	if (sock.so_uidinfo == NULL)
		return (-1);

	if (getbuf((u_long) sock.so_uidinfo, &uidinfo, sizeof(uidinfo)) == -1)
		return (-1);

	return (uidinfo.ui_uid);
#else
	return (sock.so_uid);
#endif
}

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
	nat_t *np;
	nat_t nat;
	char os[24];
	char user[MAX_ULEN];
	struct sockaddr_storage ss;

	/*
	** Only IPv4 is supported right now..
	*/

	if (faddr->ss_family != AF_INET || laddr->ss_family != AF_INET)
		return (-1);

	if (getbuf(kinfo->nl[N_NATLIST].n_value, &np, sizeof(np)) == -1)
		return (-1);

	for (; np != NULL ; np = nat.nat_next) {
		int ret;
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

		if (opt_enabled(FORWARD)) {
			ret = fwd_request(sock, lport, masq_lport, fport, masq_fport, &ss);
			if (ret == 0)
				return (0);
			else {
				char ipbuf[MAX_IPLEN];

				get_ip(&ss, ipbuf, sizeof(ipbuf));
				debug("Forward to %s (%d %d) failed",
					ipbuf, nat.nat_inport, fport);
			}
		}

		ret = find_masq_entry(&ss, user, sizeof(user), os, sizeof(os));
		if (ret == 0) {
			char ipbuf[MAX_IPLEN];

			sockprintf(sock, "%d , %d : USERID : %s : %s\r\n",
				lport, fport, os, user);

			get_ip(faddr, ipbuf, sizeof(ipbuf));

			o_log(NORMAL,
				"[%s] (NAT) Successful lookup: %d , %d : %s",
				ipbuf, lport, fport, user);

			return (0);
		}
	}

	return (-1);
}

#endif

#ifdef WANT_IPV6

/*
** Traverse the tcb6 list until a match is found.
** Returns NULL if no match.
*/

#if __NetBSD_Version__ >= 106250000
static struct socket *getlist6(	struct inpcbtable *tcbtablep,
								struct inpcbtable *ktcbtablep,
#else
static struct socket *getlist6(	struct in6pcb *tcb6,
#endif
								in_port_t lport,
								in_port_t fport,
								const struct in6_addr *laddr,
								const struct in6_addr *faddr)
{
#if __NetBSD_Version__ >= 106250000
	struct in6pcb *kpcbp, pcb;

	if (tcbtablep == NULL)
		return (NULL);

	kpcbp = (struct in6pcb *) tcbtablep->inpt_queue.cqh_first;
	while (kpcbp != (struct in6pcb *) ktcbtablep) {
		if (getbuf((u_long) kpcbp, &pcb, sizeof(struct in6pcb)) == -1)
			break;
		if (pcb.in6p_fport == fport &&
			pcb.in6p_lport == lport &&
			IN6_ARE_ADDR_EQUAL(&pcb.in6p_laddr, laddr) &&
			IN6_ARE_ADDR_EQUAL(&pcb.in6p_faddr, faddr))
		{
			return (pcb.in6p_socket);
		}

		kpcbp = (struct in6pcb *) pcb.in6p_queue.cqe_next;
	}
#else
	struct in6pcb *tcb6_cur, tcb6_temp;

	if (tcb6 == NULL)
		return (NULL);

	tcb6_cur = tcb6;

	memcpy(&tcb6_temp, tcb6, sizeof(tcb6_temp));

	do {
		if (tcb6_temp.in6p_fport == fport &&
			tcb6_temp.in6p_lport == lport &&
			IN6_ARE_ADDR_EQUAL(&tcb6_temp.in6p_laddr, laddr) &&
			IN6_ARE_ADDR_EQUAL(&tcb6_temp.in6p_faddr, faddr))
		{
			return (tcb6_temp.in6p_socket);
		}

		tcb6_cur = tcb6_temp.in6p_next;
		if (getbuf((u_long) tcb6_cur, &tcb6_temp, sizeof(tcb6_temp)) == -1)
			break;
	} while ((u_long) tcb6_cur != kinfo->nl[N_TCB6].n_value);
#endif
	return (NULL);
}

/*
** Check ident requests for NAT connection
*/

int get_user6(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
#if __NetBSD_Version__ >= 106250000	/* 1.6Y */
	struct socket *sockp, sock;
	struct inpcbtable tcbtable;
	int ret;
#ifdef SO_UIDINFO
	struct uidinfo uidinfo;
#endif

	ret = getbuf(kinfo->nl[N_TCB6].n_value, &tcbtable, sizeof(tcbtable));
	if (ret == -1)
		return (-1);

	sockp = getlist6(&tcbtable,
				(struct inpcbtable *) kinfo->nl[N_TCB6].n_value,
				lport, fport, &SIN6(laddr)->sin6_addr, &SIN6(faddr)->sin6_addr);
#else
	struct socket *sockp, sock;
	struct in6pcb tcb6;
	int ret;

	ret = getbuf(kinfo->nl[N_TCB6].n_value, &tcb6, sizeof(tcb6));
	if (ret == -1)
		return (-1);

	sockp = getlist6(&tcb6, lport, fport,
				&SIN6(laddr)->sin6_addr, &SIN6(faddr)->sin6_addr);
#endif

	if (sockp == NULL)
		return (-1);

	if (getbuf((u_long) sockp, &sock, sizeof(sock)) == -1)
		return (-1);

#ifdef SO_UIDINFO
	if (sock.so_uidinfo == NULL)
		return (-1);

	if (getbuf((u_long) sock.so_uidinfo, &uidinfo, sizeof(uidinfo)) == -1)
		return (-1);

	return (uidinfo.ui_uid);
#else
	return (sock.so_uid);
#endif
}

#endif
