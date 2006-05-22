/*
** solaris8.c - SunOS 5.8 kernel access functions
**
** Copyright (c) 1995-1999	Casper Dik <Casper.Dik@Holland.Sun.COM>
** Copyright (c) 1997		Peter Eriksson <pen@lysator.liu.se>
** Copyright (c) 2001-2006	Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or
** modify it as you wish - as long as you don't claim that you wrote
** it.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** For now, only support IPv4 for Solaris 8
*/

#include <config.h>

#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <pwd.h>

#include <stdio.h>
#include <nlist.h>
#include <math.h>

#define _KMEMUSER
#define _KERNEL

#include <kvm.h>

/* some definition conflicts. but we must define _KERNEL */

#define exit			kernel_exit
#define strsignal		kernel_strsignal
#define mutex_init		kernel_mutex_init
#define mutex_destroy	kernel_mutex_destroy
#define sema_init		kernel_sema_init
#define sema_destroy	kernel_sema_destroy

#include <syslog.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <netinet/in.h>

#include <sys/fcntl.h>
#include <sys/cred.h>
#include <sys/file.h>
#include <sys/stream.h>
#include <sys/dlpi.h>
#include <net/if_types.h>
#include <inet/common.h>
#include <netinet/ip6.h>
#include <inet/ip.h>
#include <inet/tcp.h>
#include <netinet/ip6.h>
#include <net/if.h>

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_inet_util.h>
#include <oidentd_missing.h>

typedef struct hashentry {
	tcpb_t		*he_tcp;
	kmutex_t	he_lock;
} he_t;

#define GETBYTE(x, n) ((u_int32_t) (((u_int8_t *) &x)[n]))

#define FANOUT_OFFSET(n)	(kinfo->hash_table + (n) * sizeof(he_t) + offsetof(he_t, he_tcp))

#undef exit
#undef strsignal
#undef mutex_init
#undef mutex_destroy
#undef sema_init
#undef sema_destroy

#undef SEMA_HELD
#undef RW_LOCK_HELD
#undef RW_READ_HELD
#undef RW_WRITE_HELD
#undef MUTEX_HELD

#define N_FANOUT		0
#define N_HASH_SIZE		1
#define NFPREAD			64

#ifndef NFPCHUNK
#	define uf_ofile		uf_file
#	define u_flist		u_finfo.fi_list
#	define u_nofiles	u_finfo.fi_nfiles
#endif

static struct kainfo {
	kvm_t *kd;
	int hash_size;
	u_long hash_table;
	struct nlist nl[3];
} *kinfo;

static int getbuf(kvm_t *kd, off_t addr, void *dst, size_t len);

/*
** This is needed as stdlib.h can't be included as it causes
** a clash with exit(), as declared by another header file.
*/

extern void free(void *ptr);

/*
** Open kernel devices, lookup kernel symbols etc...
*/

int k_open(void) {
	int ret;

	kinfo = xmalloc(sizeof(struct kainfo));

	/*
	** Open the kernel memory device
	*/

	kinfo->kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
	if (kinfo->kd == NULL) {
		debug("kvm_open: %s", strerror(errno));
		free(kinfo);
		return (-1);
	}

	kinfo->nl[0].n_name = "tcp_conn_fanout";
	kinfo->nl[1].n_name = "tcp_conn_hash_size";
	kinfo->nl[2].n_name = NULL;

	/*
	** Extract offsets to the needed variables in the kernel
	*/

	if (kvm_nlist(kinfo->kd, kinfo->nl) != 0) {
		debug("kvm_nlist: %s", strerror(errno));
		goto out_err;
	}

	/*
	** Read the two kernel values we need but won't change
	*/

	ret = getbuf(kinfo->kd, kinfo->nl[N_HASH_SIZE].n_value,
			&kinfo->hash_size, sizeof(kinfo->hash_size));

	if (ret == -1)
		goto out_err;

	ret = getbuf(kinfo->kd, kinfo->nl[N_FANOUT].n_value,
			&kinfo->hash_table, sizeof(kinfo->hash_table));

	if (ret == -1)
		goto out_err;

	return (0);

out_err:
	kvm_close(kinfo->kd);
	free(kinfo);
	debug("getbuf: can't get needed symbols: %s", strerror(errno));

	return (-1);
}

/*
** Get a piece of kernel memory with error handling.
** Returns 0 if call succeeded, else -1.
*/

static int getbuf(kvm_t *kd, off_t addr, void *dst, size_t len) {
	int i;
	ssize_t status = -1;

	for (i = 0 ; i < 5 ; i++) {
		status = kvm_read(kd, addr, dst, len);
		if (status >= 0)
			break;
	}

	if (status < 0)
		return (-1);

	return (0);
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
	in_addr_t laddr4 = SIN4(laddr)->sin_addr.s_addr;
	in_addr_t faddr4 = SIN4(faddr)->sin_addr.s_addr;
	u_int32_t offset;
	char *iphash;
	file_t tf;
	struct proc *procp;
	int ret;
	struct stdata *std;
	tcpb_t *tcpb;
	tcpb_t tb;
	queue_t *q;

	if (faddr->ss_family == AF_INET)
		iphash = (char *) &faddr4;
	else
#ifdef WANT_IPV6
		iphash = ((char *) &SIN6(faddr)->sin6_addr) + 12;
#else
		return (-1);
#endif

	/*
	** All tcp connections are in one single hash table; IPV4 connections
	** over AF_INET6 sockets do not show up in the normal tcp hash tables
	**
	** First we need to find the hash entry in the tcp table;
	** then we need to follow the chain and get the TCP entry.
	**
	** In Solaris 8, the tcp structure is split in two: the core part
	** needed in TIME_WAIT state and the full structure.
	*/

	offset = GETBYTE(*iphash, 3) ^ GETBYTE(fport, 0) ^ GETBYTE(fport, 1) ^
			GETBYTE(lport, 0) ^ GETBYTE(lport, 1) ^
			((GETBYTE(fport, 0) ^ GETBYTE(lport, 0) ^ GETBYTE(*iphash, 2)) << 10)
			^ (GETBYTE(*iphash, 1) << 6);

	offset %= kinfo->hash_size;

	if (getbuf(kinfo->kd, FANOUT_OFFSET(offset), &tcpb, sizeof(tcpb)) == -1)
		return (-1);

	if (tcpb == NULL)
		return (-1);

	while (tcpb != NULL) {
		if (getbuf(kinfo->kd, (off_t) tcpb, &tb, sizeof(tb)) == -1)
			return (-1);

		if (lport == tb.tcpb_lport && fport == tb.tcpb_fport) {
			if (faddr->ss_family == AF_INET) {
				struct in_addr fv4, lv4;

				sin_extractv4(&tb.tcpb_ip_src_v6, &lv4);
				sin_extractv4(&tb.tcpb_remote_v6, &fv4);

				if (!memcmp(&lv4, &laddr4, 4) && !memcmp(&fv4, &faddr4, 4))
					break;
			} else {
				size_t len_local = sin_len(laddr);
				size_t len_remote = sin_len(faddr);

				if (!memcmp(&tb.tcpb_ip_src_v6, &laddr4, len_local) &&
					!memcmp(&tb.tcpb_remote_v6, &faddr4, len_remote))
				{
					break;
				}
			}
		}

		tcpb = tb.tcpb_conn_hash;
	}

	if (tcpb == NULL)
		return (-1);

	ret = getbuf(kinfo->kd, (off_t) tb.tcpb_tcp + offsetof(tcp_t, tcp_rq),
			&q, sizeof(q));

	if (ret == -1)
		return (-1);

	ret = getbuf(kinfo->kd, (off_t) q + offsetof(queue_t, q_stream),
			&std, sizeof(std));

	if (ret == -1)
		return (-1);

	/*
	** at this point std holds the pointer to the stream we're
	** interested in. Now we're going to find the file pointer
	** that refers to the vnode that refers to this stream stream
	*/

	if (kvm_setproc(kinfo->kd) != 0)
		return (-1);

	/*
	** In Solaris 8, the file lists changed dramatically.
	** There's no longer an NFPCHUNK; the uf_entries are
	** part of a seperate structure inside user.
	*/

	while ((procp = kvm_nextproc(kinfo->kd)) != NULL) {
		struct uf_entry files[NFPREAD];
		int nfiles = procp->p_user.u_nofiles;
		off_t addr = (off_t) procp->p_user.u_flist;

		while (nfiles > 0) {
			int nread = nfiles > NFPREAD ? NFPREAD : nfiles;
			int size = nread * sizeof(struct uf_entry);
			int i;
			struct file *last = NULL;
			vnode_t vp;

			if (getbuf(kinfo->kd, addr, &files[0], size) == -1)
				return (-1);

			for (i = 0 ; i < nread ; i++) {
				if (files[i].uf_ofile == 0 || files[i].uf_ofile == last)
					continue;

				last = files[i].uf_ofile;

				if (getbuf(kinfo->kd, (off_t) last, &tf, sizeof(tf)) == -1)
					return (-1);

				if (tf.f_vnode == NULL)
					continue;

				ret = getbuf(kinfo->kd,
						(off_t) tf.f_vnode + offsetof(vnode_t, v_stream),
						&vp.v_stream,
						sizeof(vp.v_stream));

				if (ret == -1)
					return (-1);

				if (vp.v_stream == std) {
					cred_t cr;

					ret = getbuf(kinfo->kd, (off_t) tf.f_cred, &cr, sizeof(cr));
					if (ret == -1)
						return (-1);

					return (cr.cr_ruid);
				}
			}

			nfiles -= nread;
			addr += size;
		}
	}

	return (-1);
}
