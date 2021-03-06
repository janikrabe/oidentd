/*
** freebsd.c - Low level kernel access functions for FreeBSD.
**
** This file was originally taken from the pidentd 2.x software package.
** The original copyright notice is as follows:
**
**		This program is in the public domain and may be used freely
**		by anyone who wants to.
**
** The IP masquerading functionality has been taken from openbsd.c and
** is distributed under the same copyright.
**
** Modifications Copyright (c) 1998-2006 Ryan McCabe <ryan@numb.org>
** Modifications Copyright (c) 2018-2019 Janik Rabe  <info@janikrabe.com>
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <nlist.h>
#include <kvm.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <sys/socketvar.h>
#define KERNEL
#define _KERNEL
#include <sys/file.h>
#undef KERNEL
#undef _KERNEL
#include <sys/user.h>
#include <sys/filedesc.h>
#include <sys/proc.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

#include <arpa/inet.h>

#if MASQ_SUPPORT
#	include <netinet/ip_compat.h>
#	include <netinet/ip_fil.h>
#	include <netinet/ip_nat.h>
#endif

#include "oidentd.h"
#include "util.h"
#include "inet_util.h"
#include "missing.h"
#include "options.h"
#include "masq.h"

extern struct sockaddr_storage proxy;

#ifdef INPLOOKUP_SETLOCAL
#	define _HAVE_OLD_INPCB
#endif

#define N_TCB			0

#if MASQ_SUPPORT
#	define N_NATLIST	1
#	define N_TOTAL		3
#else
#	define N_TOTAL		2
#endif

static int getbuf(u_long addr, void *buf, size_t len);

static struct socket *getlist4(	void *arg,
								in_port_t lport,
								in_port_t fport,
								const struct in_addr *laddr,
								const struct in_addr *faddr);

static struct kainfo {
	kvm_t *kd;
	struct nlist nl[N_TOTAL];
} *kinfo;

/*
** Open the kernel memory device.
** Return 0 on success, or -1 with errno set.
*/

int k_open(void) {
	kinfo = xmalloc(sizeof(struct kainfo));

	kinfo->kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
	if (!kinfo->kd) {
		free(kinfo);
		debug("kvm_open: %s", strerror(errno));
		return -1;
	}

	kinfo->nl[N_TCB].n_name = "_tcb";

#if MASQ_SUPPORT
	if (opt_enabled(MASQ))
		kinfo->nl[N_NATLIST].n_name = "_nat_instances";
	else
		kinfo->nl[N_NATLIST].n_name = NULL;
#endif

	kinfo->nl[N_TOTAL - 1].n_name = NULL;

	if (kvm_nlist(kinfo->kd, kinfo->nl) != 0) {
		kvm_close(kinfo->kd);
		free(kinfo);
		debug("kvm_nlist: %s", strerror(errno));
		return -1;
	}

#if MASQ_SUPPORT
	if (opt_enabled(MASQ) && kinfo->nl[N_NATLIST].n_value == 0) {
		o_log(LOG_CRIT, "NAT/IP masquerading support is unavailable");
		disable_opt(MASQ);
	}
#endif

	return 0;
}

/*
** Get a piece of kernel memory.
** Returns 0 on success, -1 on failure.
*/

static int getbuf(u_long addr, void *buf, size_t len) {

	if (kvm_read(kinfo->kd, addr, buf, len) < 0) {
		debug("getbuf: kvm_read: %s", strerror(errno));
		return -1;
	}

	return 0;
}

/*
** Traverse the inpcb list until a match is found.
** Returns NULL if no match.
*/

#ifdef _HAVE_OLD_INPCB

static struct socket *getlist(	void *arg,
								in_port_t lport,
								in_port_t fport,
								const struct sockaddr *laddr,
								const struct sockaddr *faddr)
{
	struct inpcb *pcbp = arg;
	struct inpcb *head;

	if (!pcbp)
		return NULL;

	head = pcbp->inp_prev;

	do {
		if (opt_enabled(PROXY)) {
			if (SIN4(faddr)->sin_addr.s_addr == SIN4(&proxy)->sin_addr.s_addr &&
				SIN4(laddr)->sin_addr.s_addr != SIN4(&proxy)->sin_addr.s_addr &&
				pcbp->inp_fport == fport &&
				pcbp->inp_lport == lport)
			{
				return pcb.inp_socket;
			}
		}

		if (pcbp->inp_faddr.s_addr == SIN4(faddr)->sin_addr.s_addr &&
			pcbp->inp_laddr.s_addr == SIN4(laddr)->sin_addr.s_addr &&
			pcbp->inp_fport == fport &&
			pcbp->inp_lport == lport)
		{
			return pcbp->inp_socket;
		}
	} while (pcbp->inp_next != head &&
		getbuf((u_long) pcbp->inp_next, pcbp, sizeof(struct inpcb)) != -1);

	return NULL;
}

#else

static struct socket *getlist(	void *arg,
								in_port_t lport,
								in_port_t fport,
								const struct sockaddr *local,
								const struct sockaddr *remote)
{
	struct inpcb *head, pcbp;
	struct inpcbhead *pcbhead = arg;
	char *faddr, *laddr;

	if (remote->sa_family != local->sa_family)
		return NULL;
	switch (remote->sa_family) {
	case AF_INET:
		faddr = (char *)&SIN4(remote)->sin_addr;
		laddr = (char *)&SIN4(local)->sin_addr;
		break;
#ifdef INP_IPV6
	case AF_INET6:
		faddr = (char *)&SIN6(remote)->sin6_addr;
		laddr = (char *)&SIN6(local)->sin6_addr;
		break;
#endif
	default:
		return NULL;
	}

	head = pcbhead->lh_first;
	if (!head)
		return NULL;

	for (; head; head = pcbp.inp_list.le_next) {
		char *pfaddr, *pladdr;
		int alen;

		if (getbuf((u_long) head, &pcbp, sizeof(struct inpcb)) == -1)
			break;

		if (opt_enabled(PROXY) && remote->sa_family == AF_INET) {
			if (SIN4(remote)->sin_addr.s_addr == SIN4(&proxy)->sin_addr.s_addr &&
				SIN4(local)->sin_addr.s_addr != SIN4(&proxy)->sin_addr.s_addr &&
				pcbp.inp_fport == fport &&
				pcbp.inp_lport == lport)
			{
				return pcbp.inp_socket;
			}
		}

#ifdef INP_IPV6
		if (pcbp.inp_vflag & INP_IPV4)
		{
			if (remote->sa_family != AF_INET)
				continue;
			pfaddr = (char *)&pcbp.inp_faddr;
			pladdr = (char *)&pcbp.inp_laddr;
			alen = sizeof(struct in_addr);
		}
		else if (pcbp.inp_vflag & INP_IPV6)
		{
			if (remote->sa_family != AF_INET6)
				continue;
			pfaddr = (char *)&pcbp.in6p_faddr;
			pladdr = (char *)&pcbp.in6p_laddr;
			alen = sizeof(struct in6_addr);
		}
		else
			continue;
#else
		pfaddr = (char *)&pcbp.inp_faddr;
		pladdr = (char *)&pcbp.inp_laddr;
		alen = sizeof(struct in_addr);
#endif
		if (memcmp(pfaddr, faddr, alen) == 0 &&
			memcmp(pladdr, laddr, alen) == 0 &&
			pcbp.inp_fport == fport &&
			pcbp.inp_lport == lport)
		{
			return pcbp.inp_socket;
		}

	}

	return NULL;
}

#endif

/*
** System-dependent initialization; called only once.
** Called before privileges are dropped.
** Returns non-zero on failure.
*/

int core_init(void) {
	return 0;
}

/*
** Return the UID of the connection owner
*/

static uid_t get_user(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	struct kinfo_proc *kp;
	struct socket *sockp;
	int i;
	int nentries;
#ifdef _HAVE_OLD_INPCB
	struct inpcb tcb;
#else
	struct inpcbhead tcb;
#endif

	kp = kvm_getprocs(kinfo->kd, KERN_PROC_ALL, 0, &nentries);
	if (!kp) {
		debug("kvm_getprocs: %s", strerror(errno));
		return MISSING_UID;
	}

	if (getbuf(kinfo->nl[N_TCB].n_value, &tcb, sizeof(tcb)) == -1)
		return MISSING_UID;

#ifdef _HAVE_OLD_INPCB
	tcb.inp_prev = (struct inpcb *) kinfo->nl[N_TCB].n_value;
#endif

	sockp = getlist(&tcb, lport, fport,
				(struct sockaddr *)laddr,
				(struct sockaddr *)faddr);

	if (!sockp)
		return MISSING_UID;

	/*
	** Locate the file descriptor that has the socket in question
	** open so that we can get the 'ucred' information
	*/

	for (i = 0; i < nentries; ++i) {
		if (kp[i].kp_proc.p_fd) {
			int j;
			int ret;
			struct filedesc pfd;
			struct file **ofiles;

			if (getbuf((u_long) kp[i].kp_proc.p_fd, &pfd, sizeof(pfd)) == -1)
				return MISSING_UID;

			ofiles = xmalloc(pfd.fd_nfiles * sizeof(struct file *));

			ret = getbuf((u_long) pfd.fd_ofiles, ofiles,
					pfd.fd_nfiles * sizeof(struct file *));

			if (ret == -1) {
				free(ofiles);
				return MISSING_UID;
			}

			for (j = 0; j < pfd.fd_nfiles; ++j) {
				struct file ofile;

				if (!ofiles[j])
					continue;

				ret = getbuf((u_long) ofiles[j], &ofile, sizeof(struct file));
				if (ret == -1) {
					free(ofiles);
					return MISSING_UID;
				}

				if (ofile.f_count == 0)
					continue;

				if (ofile.f_type == DTYPE_SOCKET &&
					(struct socket *) ofile.f_data == sockp)
				{
					struct pcred pc;

					ret = getbuf((u_long) kp[i].kp_proc.p_cred,
							&pc, sizeof(pc));
					if (ret == -1) {
						free(ofiles);
						return MISSING_UID;
					}

					free(ofiles);
					return pc.p_ruid;
				}
			}

			free(ofiles);
		}
	}

	return MISSING_UID;
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
	return get_user(lport, fport, laddr, faddr);
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
	nat_t *np;
	nat_t nat;
	char os[24];
	char user[MAX_ULEN];
	struct sockaddr_storage ss;

	/*
	** Only IPv4 is supported right now..
	*/

	if (faddr->ss_family != AF_INET || laddr->ss_family != AF_INET)
		return -1;

	if (getbuf(kinfo->nl[N_NATLIST].n_value, &np, sizeof(np)) == -1)
		return -1;

	for (; np; np = nat.nat_next) {
		in_port_t masq_lport;
		in_port_t masq_fport;
		int retm;

		if (getbuf((u_long) np, &nat, sizeof(nat)) == -1) {
			debug("getbuf: %s", strerror(errno));
			break;
		}

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
			int retf;

			retf = fwd_request(sock, lport, masq_lport, fport, masq_fport, &ss);

			if (retf == 0) {
				if (retm != 0)
					return 0;
			} else {
				char ipbuf[MAX_IPLEN];

				get_ip(&ss, ipbuf, sizeof(ipbuf));

				debug("Forward to %s (%d %d | %d %d) failed",
					ipbuf, lport, fport, nat.nat_inport, nat.nat_outport);
			}
		}

		if (retm == 0) {
			char ipbuf[MAX_IPLEN];

			sockprintf(sock, "%d,%d:USERID:%s:%s\r\n",
				lport, fport, os, user);

			get_ip(faddr, ipbuf, sizeof(ipbuf));

			o_log(LOG_INFO,
				"[%s] (NAT) Successful lookup: %d , %d : %s",
				ipbuf, lport, fport, user);

			return 0;
		}
	}

	return -1;
}

#endif

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
	return get_user(lport, fport, laddr, faddr);
}

#endif
