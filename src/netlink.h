/*
** oidentd_netlink.h - Linux netlink definitions.
**
** This header file extracts the needed structure definitions, #defines
** and macros from linux/netlink.h and linux/tcp_diag.h, and converts them
** to be usable from userland code.
**
** Cleanup and conversion Copyright (c) 2002-2018 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __OIDENTD_NETLINK_H
#define __OIDENTD_NETLINK_H

#define NETLINK_TCPDIAG	4
#define TCPDIAG_GETSOCK	18

#define NLMSG_ERROR		0x2
#define NLMSG_DONE		0x3

#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len) ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
	(struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))

/* Socket identity */
struct tcpdiag_sockid {
	u_int16_t tcpdiag_sport;
	u_int16_t tcpdiag_dport;
	u_int32_t tcpdiag_src[4];
	u_int32_t tcpdiag_dst[4];
	u_int32_t tcpdiag_if;
	u_int32_t tcpdiag_cookie[2];
#define TCPDIAG_NOCOOKIE (~0U)
};

/* Request structure */

struct tcpdiagreq {
	u_int8_t tcpdiag_family;
	u_int8_t tcpdiag_src_len;
	u_int8_t tcpdiag_dst_len;
	u_int8_t tcpdiag_ext;
	struct tcpdiag_sockid id;
	u_int32_t tcpdiag_states;
	u_int32_t tcpdiag_dbs;
};

struct tcpdiagmsg {
	u_int8_t tcpdiag_family;
	u_int8_t tcpdiag_state;
	u_int8_t tcpdiag_timer;
	u_int8_t tcpdiag_retrans;
	struct tcpdiag_sockid id;
	u_int32_t tcpdiag_expires;
	u_int32_t tcpdiag_rqueue;
	u_int32_t tcpdiag_wqueue;
	u_int32_t tcpdiag_uid;
	u_int32_t tcpdiag_inode;
};

#endif
