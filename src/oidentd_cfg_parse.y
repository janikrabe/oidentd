%{
/*
** oidentd_cfg_parse.y - oidentd configuration parser.
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
**
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
#include <netinet/in.h>
#include <arpa/inet.h>

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_missing.h>
#include <oidentd_inet_util.h>
#include <oidentd_user_db.h>
#include <oidentd_options.h>

extern struct user_info *default_user;
extern u_int32_t current_line;
extern int parser_mode;

static FILE *open_user_config(const struct passwd *pw);
static int extract_port_range(const char *token, struct port_range *range);
static void free_cap_entries(struct user_cap *free_cap);
static void yyerror(const char *err);

static struct user_info *cur_user;
static struct user_cap *cur_cap;
list_t *pref_list;

u_int16_t default_caps;

%}

%union {
	int value;
	char *string;
}

%token TOK_USER
%token TOK_DEFAULT
%token TOK_GLOBAL
%token TOK_FROM
%token TOK_TO
%token TOK_FPORT
%token TOK_LPORT
%token TOK_FORCE
%token TOK_REPLY
%token <value> TOK_ALLOWDENY
%token <value> TOK_CAP
%token <string> TOK_STRING

%%

program:
	/* empty */
|
	program parse_global
;

parse_global:
	user_rule
|
	{
		if (parser_mode != PARSE_USER) {
			o_log(NORMAL,
				"[line %d] This construct is valid only for user configuration files", current_line);
			YYABORT;
		}

		cur_cap = xcalloc(1, sizeof(struct user_cap));
		cur_cap->caps = default_caps;
	} user_range_rule {
		list_prepend(&pref_list, cur_cap);
	}
;

user_rule:
	user_statement
|
	default_statement
;

default_statement:
	TOK_DEFAULT {
		if (parser_mode != PARSE_SYSTEM)
			YYABORT;

		cur_user = xmalloc(sizeof(struct user_info));
		cur_user->cap_list = NULL;

		user_db_set_default(cur_user);
	} '{' target_rule '}'
;

user_statement:
	TOK_USER TOK_STRING {
		if (parser_mode != PARSE_SYSTEM) {
			free($2);
			YYABORT;
		}

		cur_user = xmalloc(sizeof(struct user_info));
		cur_user->cap_list = NULL;

		if (find_user($2, &cur_user->user) == -1) {
			o_log(NORMAL, "[line %u] Invalid user: \"%s\"", current_line, $2);
			free($2);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		if (user_db_lookup(cur_user->user) != NULL) {
			o_log(NORMAL,
				"[line %u] User \"%s\" already has a capability entry",
				current_line, $2);
			free($2);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free($2);
	} '{' target_rule '}'
	{
		user_db_add(cur_user);
	}
;

target_rule:
	target_statement
|
	target_rule target_statement
;

target_statement:
	{
		cur_cap = xcalloc(1, sizeof(struct user_cap));
		cur_cap->caps = default_caps;
	} range_rule {
		list_prepend(&cur_user->cap_list, cur_cap);
	}
;

range_rule:
	TOK_DEFAULT '{' cap_rule '}' {
		if (cur_user == default_user)
			default_caps = cur_cap->caps;
	}
|
	dest_rule '{' cap_rule '}'
|
	src_rule '{' cap_rule '}'
|
	dest_rule src_rule '{' cap_rule '}'
;

src_rule:
	from_statement
|
	lport_statement
|
	from_statement lport_statement
;

dest_rule:
	to_statement
|
	fport_statement
|
	to_statement fport_statement
;

to_statement:
	TOK_TO TOK_STRING {
		cur_cap->dest = xmalloc(sizeof(struct sockaddr_storage));

		if (get_addr($2, cur_cap->dest) == -1) {
			if (parser_mode == PARSE_SYSTEM) {
				o_log(NORMAL, "[line %u]: Bad address: \"%s\"",
					current_line, $2);
			}

			free($2);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free($2);
	}
;

fport_statement:
	TOK_FPORT TOK_STRING {
		cur_cap->fport = xmalloc(sizeof(struct port_range));

		if (extract_port_range($2, cur_cap->fport) == -1) {
			if (parser_mode == PARSE_SYSTEM)
				o_log(NORMAL, "[line %u] Bad port: \"%s\"", current_line, $2);

			free($2);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free($2);
	}
;

from_statement:
	TOK_FROM TOK_STRING {
		cur_cap->src = xmalloc(sizeof(struct sockaddr_storage));

		if (get_addr($2, cur_cap->src) == -1) {
			if (parser_mode == PARSE_SYSTEM) {
				o_log(NORMAL, "[line %u]: Bad address: \"%s\"",
					current_line, $2);
			}

			free($2);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free($2);
	}
;

lport_statement:
	TOK_LPORT TOK_STRING {
		cur_cap->lport = xmalloc(sizeof(struct port_range));

		if (extract_port_range($2, cur_cap->lport) == -1) {
			if (parser_mode == PARSE_SYSTEM)
				o_log(NORMAL, "[line %u] Bad port: \"%s\"", current_line, $2);

			free($2);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free($2);
	}
;

cap_rule:
	cap_statement
|
	cap_rule cap_statement
;

force_reply:
	TOK_FORCE TOK_REPLY TOK_STRING {
		cur_cap->caps = CAP_REPLY;
		cur_cap->action = ACTION_FORCE;
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = $3;
	}
|
	force_reply TOK_STRING {
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = $2;
	}
;

cap_statement:
	TOK_FORCE TOK_CAP {
		cur_cap->caps = $2;
		cur_cap->action = ACTION_FORCE;
	}
|
	force_reply
|
	TOK_ALLOWDENY TOK_CAP {
		if ($1 == ACTION_ALLOW)
			cur_cap->caps |= $2;
		else
			cur_cap->caps &= ~$2;

		cur_cap->action = $1;
	}
;

user_range_rule:
	TOK_GLOBAL '{' user_cap_rule '}'
|
	dest_rule '{' user_cap_rule '}'
|
	src_rule '{' user_cap_rule '}'
|
	dest_rule src_rule '{' user_cap_rule '}'
;

user_reply:
	TOK_REPLY TOK_STRING {
		cur_cap->caps = CAP_REPLY;
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = $2;
	}
|
	user_reply TOK_STRING {
		if (cur_cap->num_replies < MAX_RANDOM_REPLIES) {
			cur_cap->force_data = xrealloc(cur_cap->force_data,
				++cur_cap->num_replies * sizeof(u_char *));
			cur_cap->force_data[cur_cap->num_replies - 1] = $2;
		}
	}
;

user_cap_rule:
	TOK_CAP {
		if ($1 == CAP_SPOOF || $1 == CAP_SPOOF_ALL || $1 == CAP_SPOOF_PRIVPORT)
		{
			free_cap_entries(cur_cap);
			YYABORT;
		}

		cur_cap->caps = $1;
	}
|
	user_reply
;

%%

/*
** Read in the system-wide configuration file.
*/

int read_config(const char *path) {
	FILE *fp;
	int ret;

	fp = fopen(path, "r");
	if (fp == NULL) {
		if (errno == ENOENT) {
			/*
			** If a configuration file is specified on the
			** command line, return an error if it can't be opened,
			** even if it doesn't exist.
			*/

			if (!strcmp(path, CONFFILE)) {
				struct user_info *temp_default;

				temp_default = user_db_create_default();
				user_db_set_default(temp_default);
				return (0);
			}
		}

		o_log(NORMAL, "Error opening config file: %s: %s",
			path, strerror(errno));
		return (-1);
	}

	yyrestart(fp);
	current_line = 1;
	parser_mode = PARSE_SYSTEM;
	ret = yyparse();

	fclose(fp);

	/*
	** Make sure there's a default to fall back on.
	*/

	if (default_user == NULL) {
		struct user_info *temp_default;

		temp_default = user_db_create_default();
		user_db_set_default(temp_default);
	}

	return (ret);
}

/*
** Open the user's ~/.oidentd_config file for reading
** by the parser.
*/

static FILE *open_user_config(const struct passwd *pw) {
	FILE *fp;

	fp = safe_open(pw, USER_CONF);
	if (fp == NULL)
		return (NULL);

	yyrestart(fp);
	current_line = 1;
	parser_mode = PARSE_USER;

	return (fp);
}

/*
** Read in a user's $HOME/.oidentd.conf file.
*/

list_t *user_db_get_pref_list(const struct passwd *pw) {
	FILE *fp;
	int ret;

	fp = open_user_config(pw);
	if (fp == NULL)
		return (NULL);

	cur_cap = NULL;
	pref_list = NULL;

	ret = yyparse();
	fclose(fp);

	if (ret != 0) {
		list_destroy(pref_list, user_db_cap_destroy_data);
		return (NULL);
	}

	return (pref_list);
}

static void yyerror(const char *err) {

	if (parser_mode == PARSE_USER)
		free_cap_entries(cur_cap);
	else
		o_log(NORMAL, "[line %u] %s", current_line, err);
}

/*
** Extract a port range from a token.
*/

static int extract_port_range(const char *token, struct port_range *range) {
	char *p;

	p = strchr(token, ':');
	if (p != NULL)
		*p++ = '\0';

	if (*token == '\0')
		range->min = PORT_MIN;
	else if (get_port(token, &range->min) == -1)
		return (-1);

	if (p == NULL)
		range->max = range->min;
	else {
		if (*p == '\0')
			range->max = PORT_MAX;
		else if (get_port(p, &range->max) == -1)
			return (-1);
	}

	return (0);
}

static void free_cap_entries(struct user_cap *free_cap) {
	user_db_cap_destroy_data(free_cap);

	if (free_cap != cur_cap)
		free(cur_cap);

	free(free_cap);
	free(cur_user);

	cur_cap = NULL;
	cur_user = NULL;
	pref_list = NULL;
}
