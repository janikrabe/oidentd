%{
/*
** oidentd_cfg_scan.l - oidentd configuration scanner.
** Copyright (C) 2001-2003 Ryan McCabe <ryan@numb.org>
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
#include <netdb.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_user_db.h>
#include <oidentd_options.h>
#include <oidentd_cfg_parse.h>

extern int parser_mode;

#define MAX_STRING_LEN 8192

static char string_buf[MAX_STRING_LEN];
static size_t str_idx;

static char get_esc_char(char c);

u_int32_t current_line;

%}

%option case-insensitive
%option never-interactive
%option noyywrap
%option nounput

%x state_comment
%x state_string

TO					to
FROM				from
USER				user
DEFAULT				default
GLOBAL				global
FPORT				fport
LPORT				lport
ALLOW				allow
DENY				deny
FORCE				force
HIDE				hide
REPLY				reply
RANDOM				random
NUMERIC				numeric
RANDOM_NUMERIC		random_numeric
SPOOF				spoof
SPOOF_ALL			spoof_all
SPOOF_PRIVPORT		spoof_privport

%%

"{"|"}" {
	return (yytext[0]);
}

[ \t]+ {
	/* ignore */
}

#[^\n]* {
	/* ignore */
}

{USER} {
	return (TOK_USER);
}

{DEFAULT} {
	return (TOK_DEFAULT);
}

{GLOBAL} {
	return (TOK_GLOBAL);
}

{TO} {
	return (TOK_TO);
}

{FROM} {
	return (TOK_FROM);
}

{FPORT} {
	return (TOK_FPORT);
}

{LPORT} {
	return (TOK_LPORT);
}

{ALLOW} {
	yylval.value = ACTION_ALLOW;
	return (TOK_ALLOWDENY);
}

{DENY} {
	yylval.value = ACTION_DENY;
	return (TOK_ALLOWDENY);
}

{FORCE} {
	return (TOK_FORCE);
}

{HIDE} {
	yylval.value = CAP_HIDE;
	return (TOK_CAP);
}

{RANDOM} {
	yylval.value = CAP_RANDOM;
	return (TOK_CAP);
}

{NUMERIC} {
	yylval.value = CAP_NUMERIC;
	return (TOK_CAP);
}

{RANDOM_NUMERIC} {
	yylval.value = CAP_RANDOM_NUMERIC;
	return (TOK_CAP);
}

{SPOOF} {
	yylval.value = CAP_SPOOF;
	return (TOK_CAP);
}

{SPOOF_ALL} {
	yylval.value = CAP_SPOOF_ALL;
	return (TOK_CAP);
}

{SPOOF_PRIVPORT} {
	yylval.value = CAP_SPOOF_PRIVPORT;
	return (TOK_CAP);
}

{REPLY} {
	return (TOK_REPLY);
}

\" {
	string_buf[0] = '\0';
	str_idx = 0;

	BEGIN(state_string);
}

"/*"	BEGIN(state_comment);

<state_comment>[^*\n]* {
	/* ignore */;
}

<state_comment>"*"+[^*/\n]* {
	/* ignore */;
}

<state_comment>"*"+"/" {
	BEGIN(INITIAL);
}

<state_string>\" {
	string_buf[str_idx++] = '\0';
	yylval.string = xstrdup(string_buf);

	BEGIN(INITIAL);
	return (TOK_STRING);
}

<state_string>\n {
	if (parser_mode == PARSE_SYSTEM) {
		o_log(NORMAL, "[line %u] Error: Unterminated string constant", current_line);
	}

	return (-1);
}

<*>\n {
	current_line++;
}

<state_string>\\[0-7]{1,3} {
	u_int32_t result;

	result = strtoul(yytext + 1, NULL, 8);

	if (result > 0xff) {
		if (parser_mode == PARSE_SYSTEM) {
			o_log(NORMAL, "[line %u] Bad escape sequence: \"%s\"\n",
				current_line, yytext);
		}

		return (-1);
	}

	if (str_idx >= sizeof(string_buf) - 1) {
		if (parser_mode == PARSE_SYSTEM) {
			o_log(NORMAL, "[line %u] String too long.  Max length is %d",
				current_line, sizeof(string_buf), yytext);
		}

		return (-1);
	}

	string_buf[str_idx++] = result;
}

<state_string>\\[xX][0-9A-Fa-f]{1,2} {
	u_int32_t result;

	result = strtoul(yytext + 2, NULL, 16);

	if (str_idx >= sizeof(string_buf) - 1) {
		if (parser_mode == PARSE_SYSTEM) {
			o_log(NORMAL, "[line %u] String too long.  Max length is %d",
				current_line, sizeof(string_buf), yytext);
		}

		return (-1);
	}

	string_buf[str_idx++] = result;
}

<state_string>\\[0-9]+ {
	if (parser_mode == PARSE_SYSTEM) {
		o_log(NORMAL, "[line %u] Error: Bad escape sequence: \"%s\"\n",
			current_line, yytext);
	}

	return (-1);
}

<state_string>\\. {
	if (str_idx >= sizeof(string_buf) - 1) {
		if (parser_mode == PARSE_SYSTEM) {
			o_log(NORMAL, "[line %u] String too long.  Max length is %d",
				current_line, sizeof(string_buf), yytext);
		}

		return (-1);
	}

	string_buf[str_idx++] = get_esc_char(yytext[1]);
}

<state_string>[^\\\n\"]+ {
	size_t len = yyleng;
	char *p = yytext;

	if (str_idx + len >= sizeof(string_buf) - 1) {
		if (parser_mode == PARSE_SYSTEM) {
			o_log(NORMAL, "[line %u] String too long.  Max length is %d",
				current_line, sizeof(string_buf), yytext);
		}

		return (-1);
	}

	while (*p != '\0')
		string_buf[str_idx++] = *p++;
}

[^ \n\t\"]+ {
	yylval.string = xstrdup(yytext);
	return (TOK_USERNAME);
}

. {
	return (-1);
}

%%

/*
** Return the specified escaped character.
*/

static char get_esc_char(char c) {
	switch (c) {
		case 'n':
			return ('\n');

		case 't':
			return ('\t');

		case 'r':
			return ('\r');

		case 'f':
			return ('\f');

		case 'b':
			return ('\b');

		case 'v':
			return ('\v');

		case 'a':
			return ('\a');

		case 'e':
			return ('\e');
	}

	return (c);
}
