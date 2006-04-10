/*
** oidentd_options.h - oidentd command-line handler.
** Copyright (C) 2001-2006 Ryan McCabe <ryan@numb.org>
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
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/

#ifndef __OIDENTD_OPTIONS_H_
#define __OIDENTD_OPTIONS_H_

#define _GNU_SOURCE

#define CHANGE_UID			(1 << 0x01)
#define CHANGE_GID			(1 << 0x02)
#define HIDE_ERRORS			(1 << 0x03)
#define MASQ				(1 << 0x04)
#define FORWARD				(1 << 0x05)
#define PROXY				(1 << 0x06)
#define USEUDB				(1 << 0x07)
#define DEBUG_MSGS			(1 << 0x08)
#define QUIET				(1 << 0x09)
#define FOREGROUND			(1 << 0x0a)
#define NOSYSLOG			(1 << 0x0b)
#define STDIO				(1 << 0x0c)
#define MASQ_OVERRIDE		(1 << 0x0d)

bool opt_enabled(u_int32_t option);
void disable_opt(u_int32_t option);
int get_options(int argc, char *const argv[]);

#endif
