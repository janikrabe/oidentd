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
** Modifications Copyright (C) 1998-2006 Ryan McCabe <ryan@numb.org>
*/

#define _WANT_UCRED
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

#ifdef MASQ_SUPPORT
#	include <netinet/ip_compat.h>
#	include <netinet/ip_fil.h>
#	include <netinet/ip_nat.h>
#endif

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_inet_util.h>
#include <oidentd_missing.h>
#include <oidentd_options.h>
#include <oidentd_masq.h>

extern struct sockaddr_storage proxy;

#ifdef INPLOOKUP_SETLOCAL
#	define _HAVE_OLD_INPCB
#endif

#define N_TCB			0

#ifdef MASQ_SUPPORT
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
** Open the kernel memory device
*/

int k_open(void) {
	kinfo = xmalloc(sizeof(struct kainfo));

	kinfo->kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
	if (kinfo->kd == NULL) {
		free(kinfo);
		debug("kvm_open: %s", strerror(errno));
		return (-1);
	}

	kinfo->nl[N_TCB].n_name = "_tcb";

#ifdef MASQ_SUPPORT
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
		return (-1);
	}

#ifdef MASQ_SUPPORT
	if (kinfo->nl[N_NATLIST].n_value == 0)
		disable_opt(MASQ);
#endif

	return (0);
}

/*
** Get a piece of kernel memory.
** Returns 0 on success, -1 on failure.
*/

static int getbuf(u_long addr, void *buf, size_t len) {

	if (kvm_read(kinfo->kd, addr, buf, len) < 0) {
		debug("getbuf: kvm_read: %s", strerror(errno));
		return (-1);
	}

	return (0);
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

	if (pcbp == NULL)
		return (NULL);

	head = pcbp->inp_prev;

	do {
		if (opt_enabled(PROXY)) {
			if (SIN4(faddr)->sin_addr.s_addr == SIN4(&proxy)->sin_addr.s_addr &&
				SIN4(laddr)->sin_addr.s_addr != SIN4(&proxy)->sin_addr.s_addr &&
				pcbp->inp_fport == fport &&
				pcbp->inp_lport == lport)
			{
				return (pcb.inp_socket);
			}
		}

		if (pcbp->inp_faddr.s_addr == SIN4(faddr)->sin_addr.s_addr &&
			pcbp->inp_laddr.s_addr == SIN4(laddr)->sin_addr.s_addr &&
			pcbp->inp_fport == fport &&
			pcbp->inp_lport == lport)
		{
			return (pcbp->inp_socket);
		}
	} while (pcbp->inp_next != head &&
		getbuf((u_long) pcbp->inp_next, pcbp, sizeof(struct inpcb)) != -1);

	return (NULL);
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
	char *faddr, *laddr, *pfaddr, *pladdr;
	int alen;

	if (remote->sa_family != local->sa_family)
		return (NULL);
	switch (remote->sa_family) {
	case AF_INET:
		faddr = (char *)&SIN4(remote)->sin_addr;
		laddr = (char *)&SIN4(local)->sin_addr;
		break;
	case AF_INET6:
		faddr = (char *)&SIN6(remote)->sin6_addr;
		laddr = (char *)&SIN6(local)->sin6_addr;
		break;
	default:
		return (NULL);
	}

	head = pcbhead->lh_first;
	if (head == NULL)
		return (NULL);

	do {
		if (getbuf((u_long) head, &pcbp, sizeof(struct inpcb)) == -1)
			break;

		if (opt_enabled(PROXY) && remote->sa_family == AF_INET) {
			if (SIN4(remote)->sin_addr.s_addr == SIN4(&proxy)->sin_addr.s_addr &&
				SIN4(local)->sin_addr.s_addr != SIN4(&proxy)->sin_addr.s_addr &&
				pcbp.inp_fport == fport &&
				pcbp.inp_lport == lport)
			{
				return (pcbp.inp_socket);
			}
		}

		if (remote->sa_family == AF_INET)
		{
			pfaddr = (char *)&pcbp.inp_faddr;
			pladdr = (char *)&pcbp.inp_laddr;
			alen = sizeof(struct in_addr);
		}
		else if (remote->sa_family == AF_INET6)
		{
			pfaddr = (char *)&pcbp.in6p_faddr;
			pladdr = (char *)&pcbp.in6p_laddr;
			alen = sizeof(struct in6_addr);
		}
		else
			continue;
		if (memcmp(pfaddr, faddr, alen) == 0 &&
			memcmp(pladdr, laddr, alen) == 0 &&
			pcbp.inp_fport == fport &&
			pcbp.inp_lport == lport)
		{
			return (pcbp.inp_socket);
		}

		head = pcbp.inp_list.le_next;
	} while (head != NULL);

	return (NULL);
}

#endif

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

static int get_user(	in_port_t lport,
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
	if (kp == NULL) {
		debug("kvm_getprocs: %s", strerror(errno));
		return (-1);
	}

	if (getbuf(kinfo->nl[N_TCB].n_value, &tcb, sizeof(tcb)) == -1)
		return (-1);

#ifdef _HAVE_OLD_INPCB
	tcb.inp_prev = (struct inpcb *) kinfo->nl[N_TCB].n_value;
#endif

	sockp = getlist(&tcb, lport, fport,
				(struct sockaddr *)laddr,
				(struct sockaddr *)faddr);

	if (sockp == NULL)
		return (-1);

	/*
	** Locate the file descriptor that has the socket in question
	** open so that we can get the 'ucred' information
	*/
	for (i = 0 ; i < nentries ; i++) {

		if ( kp[i].ki_fd != NULL) {
			int j;
			int ret;
			struct filedesc pfd;
			struct file **ofiles;
			if (getbuf((u_long) kp[i].ki_fd, &pfd, sizeof(pfd)) == -1)
				return (-1);
			ofiles = xmalloc(pfd.fd_nfiles * sizeof(struct file *));

			ret = getbuf((u_long) pfd.fd_ofiles, ofiles,
					pfd.fd_nfiles * sizeof(struct file *));

			if (ret == -1) {
				free(ofiles);
				return (-1);
			}

			for (j = 0 ; j < pfd.fd_nfiles ; j++) {
				struct file ofile;

				if (ofiles[j] == NULL)
					continue;

				ret = getbuf((u_long) ofiles[j], &ofile, sizeof(struct file));
				if (ret == -1) {
					free(ofiles);
					return (-1);
				}

				if (ofile.f_count == 0)
					continue;

				if (ofile.f_type == DTYPE_SOCKET &&
					(struct socket *) ofile.f_data == sockp)
				{

					free(ofiles);

					return (kp[i].ki_ruid);
				}
			}

			free(ofiles);
		}
		
	}

	return (-1);
}

int get_user4(	in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr)
{
	return (get_user(lport, fport, laddr, faddr));
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

		sin_setv4(nat.nat_inip.s_addr, &ss);

		if (opt_enabled(FORWARD)) {
			ret = fwd_request(sock, lport, masq_lport, fport, &ss);

			if (ret == 0)
				return (0);
			else {
				char ipbuf[MAX_IPLEN];

				get_ip(&ss, ipbuf, sizeof(ipbuf));

				debug("Forward to %s (%d %d | %d %d) failed",
					ipbuf, lport, fport, nat.nat_inport, nat.nat_outport);
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
	return (get_user(lport, fport, laddr, faddr));
}

#endif
