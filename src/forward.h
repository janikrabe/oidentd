/*
** forward.h - oidentd request forwarding.
** Copyright (c) 1998-2006 Ryan McCabe <ryan@numb.org>
** Copyright (c) 2018      Janik Rabe  <info@janikrabe.com>
** Copyright (c) 2018      Jan Steffens <jan.steffens@gmail.com>
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

#ifndef __OIDENTD_FORWARD_H
#define __OIDENTD_FORWARD_H

int forward_request(const struct sockaddr_storage *host,
					in_port_t port,
					in_port_t lport,
					in_port_t fport,
					char *reply,
					size_t len);

#endif
