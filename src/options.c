/*
** oidentd_options.c - oidentd command-line handler.
** Copyright (c) 2001-2006 Ryan McCabe <ryan@numb.org>
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

#include "oidentd.h"
#include "util.h"
#include "missing.h"
#include "inet_util.h"
#include "user_db.h"
#include "options.h"

#if MASQ_SUPPORT
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
extern struct sockaddr_storage **addr;
extern uid_t target_uid;
extern gid_t target_gid;

static void print_usage(void);
static void print_version_str(const char *desc, const char *val);
static void print_version_bool(const char *desc, const bool val);
static void print_version(bool verbose);
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
#if HAVE_LIBUDB
	{"udb",					no_argument,		0, 'U'},
#endif
	{"version",				no_argument,		0, 'v'},
#if MASQ_SUPPORT
	{"forward",				optional_argument,	0, 'f'},
	{"masquerade",				no_argument,		0, 'm'},
	{"masquerade-first",			no_argument,		0, 'M'},
	{"forward-last",			no_argument,		0, '0'}, /* deprecated */
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
	unsigned int naddrs = 0;

#if MASQ_SUPPORT
	if (get_port(DEFAULT_FPORT, &fwdport) == -1) {
		o_log(LOG_CRIT, "Fatal: Bad port: \"%s\"", DEFAULT_FPORT);
		return (-1);
	}
#endif

	connection_limit = 0xffffffff;

	config_file = xstrdup(CONFFILE);
	temp_os = xstrdup("UNIX");

	if (get_port(DEFAULT_PORT, &listen_port) == -1) {
		o_log(LOG_CRIT, "Fatal: Bad port: \"%s\"", DEFAULT_PORT);
		return (-1);
	}

	while ((opt = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != EOF) {
		switch (opt) {
			case 'a': {
				struct sockaddr_storage *temp_ss =
					xmalloc(sizeof(struct sockaddr_storage));

				if (naddrs % 16 == 0)
					addr = xrealloc(addr, sizeof(struct sockaddr_storage *) * (naddrs + 16));

				if (get_addr(optarg, temp_ss) == -1) {
					o_log(LOG_CRIT, "Fatal: Unknown host: \"%s\"", optarg);
					free(temp_ss);
					return (-1);
				}

				addr[naddrs++] = temp_ss;
				break;
			}

			case 'c':
				if (charset)
					free(charset);
				charset = xstrdup(optarg);
				break;

			case 'C':
				if (config_file)
					free(config_file);
				config_file = xstrdup(optarg);
				break;

			case 'd':
				enable_opt(DEBUG_MSGS);
#if !ENABLE_DEBUGGING
				o_log(LOG_CRIT, "Fatal: oidentd was compiled without debugging support");
				return (-1);
#endif
				break;

			case 'e':
				enable_opt(HIDE_ERRORS);
				break;

#if MASQ_SUPPORT
			case 'f':
			{
				const char *p = optarg ? optarg : DEFAULT_FPORT;

				if (get_port(p, &fwdport) == -1) {
					o_log(LOG_CRIT, "Fatal: Bad port: \"%s\"", p);
					return (-1);
				}

				enable_opt(MASQ | FORWARD);
				break;
			}

			case 'm':
				enable_opt(MASQ);
				break;

			case '0':
				o_log(LOG_CRIT, "Warning: The --forward-last flag is deprecated "
				                "and will be removed in the future. Please use "
				                "--masquerade-first (-M) instead.");
				/* fall through */

			case 'M':
				enable_opt(MASQ | FORWARD | MASQ_OVERRIDE);
				break;

#endif
			case 'P':
			{
				if (get_addr(optarg, &proxy) == -1) {
					o_log(LOG_CRIT, "Fatal: Unknown host: \"%s\"", optarg);
					return (-1);
				}

				enable_opt(PROXY);
				break;
			}

			case 'g':
				enable_opt(CHANGE_GID);
				if (!find_group(optarg, &target_gid)) {
					o_log(LOG_CRIT, "Fatal: Unknown group: \"%s\"", optarg);
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
					o_log(LOG_CRIT, "Fatal: Not a valid number: \"%s\"", optarg);
					return (-1);
				}

				connection_limit = temp_limit;
				break;
			}

			case 'o':
				if (temp_os)
					free(temp_os);

				if (optarg) {
					char *p;

					temp_os = xstrdup(optarg);

					p = strchr(temp_os, '\n');
					if (p)
						*p = '\0';

					p = strchr(temp_os, '\r');
					if (p)
						*p = '\0';
				} else
					temp_os = xstrdup("OTHER");

				break;

			case 'p':
				if (get_port(optarg, &listen_port) == -1) {
					o_log(LOG_CRIT, "Fatal: Bad port: \"%s\"", optarg);
					return (-1);
				}
				break;

			case 'q':
				enable_opt(QUIET);
				break;

			case 'r':
				if (failuser)
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
					o_log(LOG_CRIT, "Fatal: Bad timeout value: \"%s\"", optarg);
					return (-1);
				}
				break;
			}

			case 'u':
				enable_opt(CHANGE_UID);
				if (!find_user(optarg, &target_uid)) {
					o_log(LOG_CRIT, "Fatal: Unknown user: \"%s\"", optarg);
					return (-1);
				}
				break;

#if HAVE_LIBUDB
			case 'U':
				o_log(LOG_CRIT, "Warning: UDB support is deprecated and will "
				                "be removed in the future.");
				enable_opt(USEUDB);
				break;
#endif

			case 'v':
				print_version(true);
				exit(EXIT_SUCCESS);

			case 'h':
				print_usage();
				exit(EXIT_SUCCESS);
			default:
				print_usage();
				return (-1);
		}
	}

	if (addr)
		addr[naddrs] = NULL;

	if (charset) {
		size_t len = strlen(temp_os) + strlen(charset) + 2;

		ret_os = xmalloc(len);
		snprintf(ret_os, len, "%s,%s", temp_os, charset);
		free(temp_os);
		free(charset);
	} else {
		ret_os = temp_os;
	}

	if (opt_enabled(DEBUG_MSGS) && opt_enabled(QUIET)) {
		o_log(LOG_CRIT, "Fatal: The debug and quiet flags are incompatible");
		return (-1);
	}

#if NEED_ROOT
	/*
	** Warn the user that privileges will not be dropped automatically.
	*/

	if (!opt_enabled(CHANGE_UID) || !opt_enabled(CHANGE_GID)) {
		o_log(LOG_CRIT, "Warning: privileges will not be dropped automatically"
		                " because " PACKAGE_NAME " needs to run as root"
		                " on this system");
	}
#else
	/*
	** If no user was specified, use a reasonable default.
	*/

	if (!opt_enabled(CHANGE_UID) && (getuid() == 0 || geteuid() == 0)) {
		enable_opt(CHANGE_UID);
		if (!find_user("oidentd", &target_uid)) {
			if (!find_user("nobody", &target_uid)) {
				o_log(NORMAL,
					"Users \"oidentd\" and \"nobody\" do "
					"not exist; using %u as default UID",
					DEFAULT_UID);

				target_uid = DEFAULT_UID;
			}
		}
	}

	/*
	** If no group was specified, use a reasonable default.
	*/

	if (!opt_enabled(CHANGE_GID) && (getgid() == 0 || getegid() == 0)) {
		enable_opt(CHANGE_GID);
		if (!find_group("oidentd", &target_gid)) {
			if (!find_group("nobody", &target_gid)) {
				if (!find_group("nogroup", &target_gid)) {
					o_log(NORMAL,
						"Groups \"oidentd\", \"nobody\" "
						"and \"nogroup\" do not exist; "
						"using %u as default GID",
						DEFAULT_GID);

					target_gid = DEFAULT_GID;
				}
			}
		}
	}
#endif

	return (0);
}

static void print_usage(void) {
	const char usage[] =
"\nUsage: oidentd [options]\n"
"-a or --address <address>    Bind to <address> (can be specified multiple times)\n"
"-c or --charset <charset>    Specify an alternate charset\n"
"-C or --config <config file> Use the specified configuration file instead of the default\n"

#if ENABLE_DEBUGGING
"-d or --debug                Enable debugging\n"
#else
"-d or --debug                Enable debugging (not available in this build)\n"
#endif

"-e or --error                Return \"UNKNOWN-ERROR\" for all errors\n"

#if MASQ_SUPPORT
"-f or --forward [<port>]     Forward requests for masqueraded hosts to the host on port <port>\n"
"-m or --masquerade           Enable support for IP masquerading\n"
"-M or --masquerade-first     Check IP masquerading file before forwarding\n"
#endif

"-P or --proxy <host>         Let <host> act as a proxy, forwarding connections to us\n"
"-g or --group <group>        Run with specified group or GID\n"
"-i or --foreground           Don't run as a daemon\n"
"-I or --stdio                Service a single client connected to stdin/stdout, then exit (use with inetd/xinetd/etc.)\n"
"-l or --limit <number>       Limit the number of open connections to the specified number\n"
"-o or --other [<os>]         Return <os> instead of the operating system.  Uses \"OTHER\" if no argument is given.\n"
"-p or --port <port>          Listen for connections on specified port\n"
"-q or --quiet                Suppress normal logging\n"
"-S or --nosyslog             Write messages to stderr instead of syslog\n"
"-t or --timeout <seconds>    Wait at most <seconds> before closing connections\n"
"-u or --user <user>          Run as specified user or UID\n"

#if HAVE_LIBUDB
"-U or --udb                  Perform lookups in UDB shared memory tables (deprecated)\n"
#endif

"-v or --version              Display version information and exit\n"
"-r or --reply <string>       If a query fails, pretend it succeeded, returning <string>\n"
"-h or --help                 Display this help and exit\n";

	print_version(false);
	printf("%s", usage);
}

static inline void print_version_str(const char *desc, const char *val) {
	printf("\t%s: %s\n", desc, val);
}

static inline void print_version_bool(const char *desc, const bool val) {
	print_version_str(desc, val ? "Yes" : "No");
}

static void print_version(bool verbose) {
	printf("%s by %s <%s>\n", PACKAGE_STRING,
			PACKAGE_AUTHOR, PACKAGE_BUGREPORT);
	printf("Originally written by %s <%s>\n",
			PACKAGE_ORIG_AUTHOR, PACKAGE_ORIG_EMAIL);
	printf("%s\n", PACKAGE_WEBSITE);

	if (verbose) {
		printf("\nBuild information:\n");
		print_version_str ("Kernel driver", KERNEL_DRIVER);
		print_version_bool("Needs kmem access", USE_KMEM);
		print_version_bool("Needs root access", NEED_ROOT);
		print_version_bool("Debug build", ENABLE_DEBUGGING);
		print_version_bool("Masquerading support", MASQ_SUPPORT);
		print_version_bool("IPv6 support", WANT_IPV6);
		print_version_bool("Linux libcap-ng support", HAVE_LIBCAP_NG);
		print_version_bool("Linux libnfct support", LIBNFCT_SUPPORT);
		print_version_bool("UDB library support", HAVE_LIBUDB);

		printf("\nBuild settings:\n");
		print_version_str("Configuration directory", SYSCONFDIR);
		print_version_str("User configuration file", USER_CONF);
	}
}
