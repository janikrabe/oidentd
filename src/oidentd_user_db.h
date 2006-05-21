/*
** oidentd_user_db.h - oidentd user database routines.
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

#ifndef __OIDENTD_USER_DB_H
#define __OIDENTD_USER_DB_H

#define PARSE_USER				0x00
#define PARSE_SYSTEM			0x01

#define ACTION_DENY				0x00
#define ACTION_ALLOW			0x01
#define ACTION_FORCE			0x03

#define CAP_SPOOF				(1 << 0x01)
#define CAP_SPOOF_ALL			(1 << 0x02)
#define CAP_SPOOF_PRIVPORT		(1 << 0x03)
#define CAP_HIDE				(1 << 0x04)
#define CAP_RANDOM				(1 << 0x05)
#define CAP_RANDOM_NUMERIC		(1 << 0x06)
#define CAP_NUMERIC				(1 << 0x07)
#define CAP_REPLY				(1 << 0x08)

#define DB_HASH_SIZE			32

struct user_cap {
	struct port_range {
		in_port_t	min;
		in_port_t	max;
	} *lport, *fport;

	struct sockaddr_storage *src;
	struct sockaddr_storage	*dest;

	u_int16_t	caps;
	u_int16_t	action;
	u_int8_t	num_replies;
	char		**force_data;
};

struct user_info {
	uid_t	user;
	list_t	*cap_list;
};

struct user_info *user_db_lookup(uid_t uid);
void user_db_add(struct user_info *user_info);
void user_db_destroy(void);
void user_db_cap_destroy_data(void *data);
void user_db_set_default(struct user_info *user_info);
struct user_info *user_db_create_default(void);

int get_ident(	const struct passwd *pwd,
				in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr,
				char *reply,
				size_t len);

list_t *user_db_get_pref_list(const struct passwd *pw);

#endif
