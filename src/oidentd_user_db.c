/*
** oidentd_user_db.c - oidentd user database routines.
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

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <syslog.h>
#include <pwd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_missing.h>
#include <oidentd_inet_util.h>
#include <oidentd_user_db.h>
#include <oidentd_options.h>

#define USER_DB_HASH(x) ((x) % DB_HASH_SIZE)

int parser_mode;
struct user_cap *pref_cap;

static list_t *user_hash[DB_HASH_SIZE];
struct user_info *default_user;

static u_char *select_reply(const struct user_cap *user);
static void db_destroy_user_cb(void *data);

static bool port_match(in_port_t port, const struct port_range *cap_ports);
static bool addr_match(	struct sockaddr_storage *addr,
						struct sockaddr_storage *cap_addr);

static bool user_db_have_cap(	const struct user_cap *user_cap,
								u_int16_t cap_flag);

static bool user_db_can_reply(	const struct user_cap *user_cap,
								const char *reply,
								uid_t con_uid,
								in_port_t fport);

static struct user_cap *user_db_cap_lookup(	struct user_info *user_info,
											in_port_t lport,
											in_port_t fport,
											struct sockaddr_storage *laddr,
											struct sockaddr_storage *faddr);

static struct user_cap *user_db_get_pref(	const struct passwd *pw,
											in_port_t lport,
											in_port_t fport,
											struct sockaddr_storage *laddr,
											struct sockaddr_storage *faddr);

/*
** Generate a pseudo-random ident response consisting of a string of "len"
** characters of the set "valid"
*/

static void random_ident(char *buf, size_t len) {
	size_t i;
	static const char valid[] =
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	for (i = 0 ; i < len - 1 ; i++)
		buf[i] = valid[randval(sizeof(valid) - 1)];

	buf[i] = '\0';
}

/*
** Select a reply randomly.
*/

static inline u_char *select_reply(const struct user_cap *user) {
	return (user->force_data[randval(user->num_replies)]);
}

/*
** Use a user's UID as the ident reply.
*/

static inline void numeric_ident(uid_t con_uid, char *buf, size_t len) {
	snprintf(buf, len, "%u", (u_int32_t) con_uid);
}

/*
** Generate an ident response of the form UPREFIXxxxxx,
** where xxxxx is a pseudo-random number between 1 and 99999
*/

static inline void random_ident_numeric(char *buf, size_t len) {
	snprintf(buf, len, "%s%u", UPREFIX, randval(100000));
}

/*
** Returns true if a user has a capabilitiy, false if it doesn't.
*/

static inline bool user_db_have_cap(const struct user_cap *user_cap,
									u_int16_t cap_flag)
{
	return ((user_cap->caps & cap_flag) != 0);
}

/*
** Stores the appropriate ident reply in "reply."
** Returns 0 if user is not hidden, -1 if the user is hidden.
*/

int get_ident(	const struct passwd *pwd,
				in_port_t lport,
				in_port_t fport,
				struct sockaddr_storage *laddr,
				struct sockaddr_storage *faddr,
				char *reply,
				size_t len)
{
	struct user_cap *user_cap;
	struct user_cap *user_pref;
	uid_t cur_uid = pwd->pw_uid;

	user_cap = user_db_cap_lookup(user_db_lookup(cur_uid),
				lport, fport, laddr, faddr);

	if (user_cap == NULL)
		user_cap = user_db_cap_lookup(default_user, lport, fport, laddr, faddr);

	if (user_cap->action == ACTION_FORCE) {
		switch (user_cap->caps) {
			case CAP_REPLY:
				xstrncpy(reply, select_reply(user_cap), len);
				break;

			case CAP_HIDE:
				return (-1);
				break;

			case CAP_RANDOM:
				random_ident(reply, MIN(12, len));
				break;

			case CAP_NUMERIC:
				numeric_ident(pwd->pw_uid, reply, len);
				break;

			case CAP_RANDOM_NUMERIC:
				random_ident_numeric(reply, len);
				break;
		}

		return (0);
	}

	user_pref = user_db_get_pref(pwd, lport, fport, laddr, faddr);
	if (user_pref != NULL) {
		u_int16_t caps = user_pref->caps;

		switch (caps) {
			case CAP_HIDE:
				if (user_db_have_cap(user_cap, CAP_HIDE) == true) {
					user_db_cap_destroy_data(user_pref);
					return (-1);
				}

				break;

			case CAP_REPLY:
			{
				char *temp_reply = select_reply(user_pref);
				bool ret;

				ret = user_db_can_reply(user_cap, temp_reply, cur_uid, fport);
				if (ret == true) {
					xstrncpy(reply, temp_reply, len);
					goto out_success;
				}

				break;
			}

			case CAP_RANDOM:
				if (user_db_have_cap(user_cap, CAP_RANDOM) == true) {
					random_ident(reply, MIN(12, len));
					goto out_success;
				}

				break;

			case CAP_NUMERIC:
				if (user_db_have_cap(user_cap, CAP_NUMERIC) == true) {
					numeric_ident(pwd->pw_uid, reply, len);
					goto out_success;
				}

				break;

			case CAP_RANDOM_NUMERIC:
				if (user_db_have_cap(user_cap, CAP_RANDOM_NUMERIC) == true) {
					random_ident_numeric(reply, len);
					goto out_success;
				}

				break;
		}

		user_db_cap_destroy_data(user_pref);
	}

	xstrncpy(reply, pwd->pw_name, len);
	return (0);

out_success:
	user_db_cap_destroy_data(user_pref);
	return (0);
}

/*
** Callback for destroying a capability list
** with list_destroy.
*/

void user_db_cap_destroy_data(void *data) {
	struct user_cap *user_cap = data;
	size_t i;

	if (data == NULL)
		return;

	free(user_cap->lport);
	free(user_cap->fport);
	free(user_cap->src);
	free(user_cap->dest);

	for (i = 0 ; i < user_cap->num_replies ; i++)
		free(user_cap->force_data[i]);

	free(user_cap->force_data);
}

/*
** Callback for destroying a user_info struct
** with list_destroy.
*/

static inline void db_destroy_user_cb(void *data) {
	struct user_info *user_info = data;

	list_destroy(user_info->cap_list, user_db_cap_destroy_data);
}

/*
** Add an entry to the hash table.
*/

inline void user_db_add(struct user_info *user_info) {
	list_prepend(&user_hash[USER_DB_HASH(user_info->user)], user_info);
}

/*
** Destroy the user capability database.
*/

void user_db_destroy(void) {
	size_t i;

	for (i = 0 ; i < DB_HASH_SIZE ; i++) {
		if (user_hash[i] != NULL) {
			list_destroy(user_hash[i], db_destroy_user_cb);
			user_hash[i] = NULL;
		}
	}

	db_destroy_user_cb(default_user);
	default_user = NULL;
}

/*
** Returns true if the user specified by "con_uid" is allowed
** to spoof "reply" to destination port "fport," otherwise
** returns false.
*/

static bool user_db_can_reply(	const struct user_cap *user_cap,
								const char *reply,
								uid_t con_uid,
								in_port_t fport)
{
	struct passwd *pw;

	pw = getpwnam(reply);
	if (pw != NULL) {
		/*
		** A user can always reply with her own username.
		*/

		if (pw->pw_uid == con_uid)
			return (true);

		if (user_db_have_cap(user_cap, CAP_SPOOF_ALL) == false) {
			o_log(LOG_INFO, "User %s tried to masquerade as user %s",
				reply, pw->pw_name);

			return (false);
		}
	}

	if (user_db_have_cap(user_cap, CAP_SPOOF) == false)
		return (false);

	if (fport < 1024 &&
		user_db_have_cap(user_cap, CAP_SPOOF_PRIVPORT) == false)
	{
		return (false);
	}

	return (true);
}

/*
** Find the entry in the hash table for the given UID.
*/

struct user_info *user_db_lookup(uid_t uid) {
	list_t *cur;

	cur = user_hash[USER_DB_HASH(uid)];
	while (cur != NULL) {
		struct user_info *user_info = cur->data;

		if (user_info->user == uid)
			return (user_info);

		cur = cur->next;
	}

	return (NULL);
}

/*
** Find the list of capabilitites for the user.
*/

static struct user_cap *user_db_cap_lookup(	struct user_info *user_info,
											in_port_t lport,
											in_port_t fport,
											struct sockaddr_storage *laddr,
											struct sockaddr_storage *faddr)
{
	list_t *cur;

	if (user_info == NULL)
		return (NULL);

	for (cur = user_info->cap_list ; cur != NULL ; cur = cur->next) {
		struct user_cap *user_cap = cur->data;

		if (port_match(lport, user_cap->lport) == false)
			continue;

		if (port_match(fport, user_cap->fport) == false)
			continue;

		if (addr_match(laddr, user_cap->src) == false)
			continue;

		if (addr_match(faddr, user_cap->dest) == false)
			continue;

		return (user_cap);
	}

	return (NULL);
}

/*
** Create the default user, if none was specified
** in the system-wide configuration.
*/

struct user_info *user_db_create_default(void) {
	struct user_info *temp_default;
	struct user_cap *cur_cap;

	temp_default = xmalloc(sizeof(struct user_info));
	temp_default->cap_list = NULL;

	cur_cap = xcalloc(1, sizeof(struct user_cap));
	list_prepend(&temp_default->cap_list, cur_cap);

	return (temp_default);
}

/*
** Sets "user_info" as the default user.
*/

void user_db_set_default(struct user_info *user_info) {
	if (default_user != NULL) {
		list_destroy(default_user->cap_list, user_db_cap_destroy_data);
		free(default_user);
	}

	default_user = user_info;
}

/*
** Find out if the user has specified any action for this range.
*/

static struct user_cap *user_db_get_pref(	const struct passwd *pw,
											in_port_t lport,
											in_port_t fport,
											struct sockaddr_storage *laddr,
											struct sockaddr_storage *faddr)
{
	list_t *cap_list;
	list_t *cur;

	cap_list = user_db_get_pref_list(pw);

	for (cur = cap_list ; cur != NULL ; cur = cur->next) {
		struct user_cap *cur_cap = cur->data;

		if (port_match(lport, cur_cap->lport) == false)
			continue;

		if (port_match(fport, cur_cap->fport) == false)
			continue;

		if (addr_match(laddr, cur_cap->src) == false)
			continue;

		if (addr_match(faddr, cur_cap->dest) == false)
			continue;

		/*
		** Don't let list_destroy destroy this one.
		*/
		cur->data = NULL;

		list_destroy(cap_list, user_db_cap_destroy_data);
		return (cur_cap);
	}

	list_destroy(cap_list, user_db_cap_destroy_data);
	return (NULL);
}

/*
** Compares two internet addresses.  NULL is treated as a
** wildcard.
*/

static bool addr_match(	struct sockaddr_storage *addr,
						struct sockaddr_storage *cap_addr)
{
	if (cap_addr == NULL)
		return (true);

	return (sin_equal(addr, cap_addr));
}

/*
** Checks whether "port" is contained in the range "cap_ports"
** NULL is treated as a wildcard.
*/

static bool port_match(in_port_t port, const struct port_range *cap_ports) {
	if (cap_ports == NULL)
		return (true);

	if (port >= cap_ports->min && port <= cap_ports->max)
		return (true);

	return (false);
}
