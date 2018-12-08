/*
** oidentd_masq.h - oidentd IP masquerading handler.
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

#ifndef __OIDENTD_MASQ_H
#define __OIDENTD_MASQ_H

#if MASQ_SUPPORT

int find_masq_entry(struct sockaddr_storage *host,
					char *user,
					size_t user_len,
					char *os,
					size_t os_len);

int fwd_request(int sock,
				in_port_t real_lport,
				in_port_t masq_lport,
				in_port_t real_fport,
				in_port_t masq_fport,
				struct sockaddr_storage *mrelay);

#endif

int masq(	int sock,
			in_port_t lport,
			in_port_t fport,
			struct sockaddr_storage *laddr,
			struct sockaddr_storage *faddr);

#endif
