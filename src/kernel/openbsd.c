/*
** openbsd.c - Low level kernel access functions for OpenBSD.
**
** This file was originally taken from the pidentd 2.x software package.
** The original copyright notice is as follows:
**
**		This program is in the public domain and may be used freely
**		by anyone who wants to.
**
** OpenBSD IP masquerading support Copyright (C) 2000
** Slawomir Piotrowski <slawek@telsatgp.com.pl>
**
** Modifications Copyright (C) 1998-2006 Ryan McCabe <ryan@numb.org>
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
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#ifdef WANT_IPV6
#	include <sys/sysctl.h>
#	include <netinet/ip_var.h>
#	include <netinet/tcp_timer.h>
#	include <netinet/tcp_var.h>
#endif

#ifdef MASQ_SUPPORT
#	include <netinet/ip_fil_compat.h>
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

#ifdef MASQ_SUPPORT
#	define N_NATLIST	1
#	define N_TOTAL		3
#else
#	define N_TOTAL		2
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

#ifdef MASQ_SUPPORT
	if (opt_enabled(MASQ))
		kinfo->nl[N_NATLIST].n_name = "_nat_instances";
	else
		kinfo->nl[N_NATLIST].n_name = "NULL";
#endif

	kinfo->nl[N_TOTAL - 1].n_name = NULL;

	if (kvm_nlist(kinfo->kd, kinfo->nl) != 0) {
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

	kpcbp = tcbtablep->inpt_queue.cqh_first;
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

		kpcbp = pcb.inp_queue.cqe_next;
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

	if (!(sock.so_state & SS_CONNECTOUT))
		return (-1);

	return (sock.so_ruid);
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

		sin_setv4(nat.nat_inip.s_addr, &ss);

		if (opt_enabled(FORWARD)) {
			ret = fwd_request(sock, lport, masq_lport, fport, &ss);
			if (ret == 0)
				return (0);
			else {
				char ipbuf[MAX_IPLEN];

				get_ip(&ss, ipbuf, sizeof(ipbuf));
				debug("Forward to %s (%d %d) failed",
					ipbuf, lport, nat.nat_inport);
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

int get_user6(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct tcp_ident_mapping tir;
	struct sockaddr_in6 *fin;
	struct sockaddr_in6 *lin;
	int mib[] = { CTL_NET, PF_INET, IPPROTO_TCP, TCPCTL_IDENT };
	int error = 0;
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
