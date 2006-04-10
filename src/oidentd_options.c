/*
** oidentd_options.c - oidentd command-line handler.
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

#define _GNU_SOURCE

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <syslog.h>
#include <getopt.h>
#include <string.h>
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

#ifdef MASQ_SUPPORT
#	define OPTSTRING "a:c:C:def::g:hiIl:mMo::p:P:qr:St:u:Uv"
	extern in_port_t fwdport;
#else
#	define OPTSTRING "a:c:C:deg:hiIl:o::p:P:qr:St:u:Uv"
#endif

extern struct sockaddr_storage proxy;
extern char *failuser;
extern char *ret_os;
extern char *config_file;
extern u_int32_t timeout;
extern u_int32_t connection_limit;
extern in_port_t listen_port;
extern struct sockaddr_storage *addr;
extern uid_t uid;
extern gid_t gid;

static void print_usage(void);
static void print_version(void);
static inline void enable_opt(u_int32_t option);

static const struct option longopts[] = {
	{"address",				required_argument,	0, 'a'},
	{"charset",				required_argument,	0, 'c'},
	{"config",				required_argument,	0, 'C'},
	{"debug",				no_argument,		0, 'd'},
	{"error",				no_argument,		0, 'e'},
	{"group",				required_argument,	0, 'g'},
	{"help",				no_argument,		0, 'h'},
	{"foreground",				no_argument,		0, 'i'},
	{"stdio",				no_argument,		0, 'I'},
	{"limit",				required_argument,	0, 'l'},
	{"other",				optional_argument,	0, 'o'},
	{"port",				required_argument,	0, 'p'},
	{"quiet",				no_argument,		0, 'q'},
	{"reply",				required_argument,	0, 'r'},
	{"nosyslog",				no_argument,		0, 'S'},
	{"timeout",				required_argument,	0, 't'},
	{"user",				required_argument,	0, 'u'},
#ifdef HAVE_LIBUDB
	{"udb",					no_argument,		0, 'U'},
#endif
	{"version",				no_argument,		0, 'v'},
#ifdef MASQ_SUPPORT
	{"forward",				optional_argument,	0, 'f'},
	{"masquerade",				no_argument,		0, 'm'},
	{"forward-last",			no_argument,		0, 'M'},
#endif
	{"proxy",				required_argument,	0, 'P'},
	{NULL, 0, NULL, 0}
};

static u_int32_t flags;

/*
** Returns true if "option" is enabled, false if it isn't.
*/

inline bool opt_enabled(u_int32_t option) {
	return ((flags & option) != 0);
}

/*
** Enables "option."
*/

static inline void enable_opt(u_int32_t option) {
	flags |= option;
}

/*
** Disables "option."
*/

inline void disable_opt(u_int32_t option) {
	flags &= ~option;
}

/*
** Parse any arguments passed to the program.
** Returns 0 on success, -1 on failure.
*/

int get_options(int argc, char *const argv[]) {
	int opt;
	char *temp_os;
	char *charset = NULL;

#ifdef MASQ_SUPPORT
	if (get_port(DEFAULT_FPORT, &fwdport) == -1) {
		o_log(NORMAL, "Fatal: Bad port: \"%s\"", DEFAULT_FPORT);
		return (-1);
	}
	fwdport = htons(fwdport);
#endif

	connection_limit = 0xffffffff;

	config_file = xstrdup(CONFFILE);
	temp_os = xstrdup("UNIX");

	if (get_port(DEFAULT_PORT, &listen_port) == -1) {
		o_log(NORMAL, "Fatal: Bad port: \"%s\"", DEFAULT_PORT);
		return (-1);
	}

	while ((opt = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != EOF) {
		switch (opt) {
			case 'a': {
				struct sockaddr_storage *temp_ss =
					xmalloc(sizeof(struct sockaddr_storage));

				if (get_addr(optarg, temp_ss) == -1) {
					o_log(NORMAL, "Fatal: Unknown host: \"%s\"", optarg);
					free(temp_ss);
					return (-1);
				}

				addr = temp_ss;
				break;
			}

			case 'c':
				if (charset != NULL)
					free(charset);
				charset = xstrdup(optarg);
				break;

			case 'C':
				if (config_file != NULL)
					free(config_file);
				config_file = xstrdup(optarg);
				break;

			case 'd':
				enable_opt(DEBUG_MSGS);
				break;

			case 'e':
				enable_opt(HIDE_ERRORS);
				break;

#ifdef MASQ_SUPPORT
			case 'f':
			{
				const char *p;

				 if (optarg == NULL)
					p = DEFAULT_FPORT;
				else
					p = optarg;

				if (get_port(p, &fwdport) == -1) {
					o_log(NORMAL, "Fatal: Bad port: \"%s\"", p);
					return (-1);
				}
				fwdport = htons(fwdport);

				enable_opt(FORWARD);
				break;
			}

			case 'm':
				enable_opt(MASQ);
				break;

			case 'M':
				enable_opt(MASQ);
				enable_opt(MASQ_OVERRIDE);
				break;

#endif
			case 'P':
			{
				if (get_addr(optarg, &proxy) == -1) {
					o_log(NORMAL, "Fatal: Unknown host: \"%s\"", optarg);
					return (-1);
				}

				enable_opt(PROXY);
				break;
			}

			case 'g':
				enable_opt(CHANGE_GID);
				if (find_group(optarg, &gid) == -1) {
					o_log(NORMAL, "Fatal: Unknown group: \"%s\"", optarg);
					return (-1);
				}
				break;

			/* pre-connected, as when run from inetd */
			case 'I':
				enable_opt(STDIO | FOREGROUND);
				break;

			/* do not become a daemon */
			case 'i':
				enable_opt(FOREGROUND);
				break;

			case 'l':
			{
				u_int32_t temp_limit;
				char *end;

				temp_limit = strtoul(optarg, &end, 10);
				if (*end != '\0') {
					o_log(NORMAL, "Fatal: Not a valid number: \"%s\"", optarg);
					return (-1);
				}

				connection_limit = temp_limit;
			}

			case 'o':
				if (temp_os != NULL)
					free(temp_os);

				if (optarg != NULL) {
					char *p;

					temp_os = xstrdup(optarg);

					p = strchr(temp_os, '\n');
					if (p != NULL)
						*p = '\0';

					p = strchr(temp_os, '\r');
					if (p != NULL)
						*p = '\0';
				} else
					temp_os = xstrdup("OTHER");

				break;

			case 'p':
				if (get_port(optarg, &listen_port) == -1) {
					o_log(NORMAL, "Fatal: Bad port: \"%s\"", optarg);
					return (-1);
				}
				break;

			case 'q':
				enable_opt(QUIET);
				break;

			case 'r':
				if (failuser != NULL)
					free(failuser);
				failuser = xstrdup(optarg);
				break;

			case 'S':
				enable_opt(NOSYSLOG);
				break;

			case 't':
			{
				char *end;

				timeout = strtoul(optarg, &end, 10);
				if (*end != '\0') {
					o_log(NORMAL, "Fatal: Bad timeout value: \"%s\"", optarg);
					return (-1);
				}
				break;
			}

			case 'u':
				enable_opt(CHANGE_UID);
				if (find_user(optarg, &uid) == -1) {
					o_log(NORMAL, "Fatal: Unknown user: \"%s\"", optarg);
					return (-1);
				}
				break;

#ifdef HAVE_LIBUDB
			case 'U':
				enable_opt(USEUDB);
				break;
#endif

			case 'v':
				print_version();
				return (-1);

			case 'h':
			default:
				print_usage();
				return (-1);
		}
	}

	if (charset != NULL) {
		size_t len = strlen(temp_os) + strlen(charset) + 4;

		ret_os = xmalloc(len);
		snprintf(ret_os, len, "%s , %s", temp_os, charset);
		free(temp_os);
		free(charset);
	} else
		ret_os = temp_os;

	if (opt_enabled(DEBUG_MSGS) && opt_enabled(QUIET)) {
		o_log(NORMAL, "Fatal: The debug and quiet flags are incompatible");
		return (-1);
	}

	/*
	** If no user was specified, use a reasonable default.
	*/

	if (!opt_enabled(CHANGE_UID)) {
		if (find_user("nobody", &uid) == -1) {
			o_log(NORMAL,
				"user \"nobody\" does not exist. Using %u as default UID",
				DEFAULT_UID);

			uid = DEFAULT_UID;
		}
	}

	/*
	** If no group is specified, use a reasonable default.
	*/

	if (!opt_enabled(CHANGE_GID)) {
		if (find_group("nobody", &gid) == -1) {
			if (find_group("nogroup", &gid) == -1) {
				o_log(NORMAL,
					"Groups \"nobody\" and \"nogroup\" do not exist. Using %u as default GID",
					DEFAULT_GID);

				gid = DEFAULT_GID;
			}
		}
	}

	return (0);
}

static void print_usage(void) {
	const char usage[] =
"\nUsage: oidentd [options]\n"
"-a or --address <address>    Bind to <address>\n"
"-c or --charset <charset>    Specify an alternate charset\n"
"-C or --config <config file> Use the specifed file instead of /etc/oidentd.conf\n"
"-d or --debug                Enable debugging\n"
"-e or --error                Return \"UNKNOWN-ERROR\" for all errors\n"

#ifdef MASQ_SUPPORT
"-f or --forward [<port>]     Forward requests for masqueraded hosts to the host on port <port>\n"
"-m or --masquerade           Enable support for IP masquerading\n"
"-M or --forward-last         Check IP masquerading file before forwarding\n"
#endif

"-P or --proxy <host>         <host> acts as a proxy, forwarding connections to us\n"
"-g or --group <group>        Run with specified group or GID\n"
"-i or --foreground           Don't run as a daemon\n"
"-I or --stdio                Service a single client connected to stdin/stdout then exit (use when oidentd is running from inetd/xinetd/etc).\n"
"-l or --limit <number>       Limit the number of open connections to the specified number\n"
"-o or --other [<os>]         Return <os> instead of the operating system.  Uses \"OTHER\" if no argument is given.\n"
"-p or --port <port>          Listen for connections on specified port\n"
"-q or --quiet                Suppress normal logging\n"
"-S or --nosyslog             Write messages to stderr not syslog\n"
"-t or --timeout <seconds>    Wait for <seconds> before closing connection\n"
"-u or --user <user>          Run as specified user or UID\n"

#ifdef HAVE_LIBUDB
"-U or --udb                  Perform lookups in UDB shared memory tables\n"
#endif

"-v or --version              Display version information and exit\n"
"-r or --reply <string>       If a query fails, pretend it succeeded, returning <string>\n"
"-h or --help                 This help message\n";

	print_version();
	printf("%s", usage);
}

static void print_version(void) {
	printf("%s\n", PACKAGE_STRING);
	printf("Written by %s <%s>\n", PACKAGE_AUTHOR, PACKAGE_BUGREPORT);
	printf("%s\n", PACKAGE_WEBSITE);
}
