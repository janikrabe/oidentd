dnl configure.ac - oidentd Ident (RFC 1413) implementation.
dnl Copyright (c) 2006      Ryan McCabe <ryan@numb.org>
dnl Copyright (c) 2018-2019 Janik Rabe  <info@janikrabe.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License, version 2,
dnl as published by the Free Software Foundation.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

AC_INIT([oidentd],[3.1.0],[info@janikrabe.com],[],[https://oidentd.janikrabe.com])
AM_INIT_AUTOMAKE([dist-bzip2 dist-xz dist-zip dist-zstd])

AC_DEFINE(PACKAGE_AUTHOR, ["Janik Rabe"], [maintainer])

AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_LEX(noyywrap)
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PATH_PROG(sedpath, sed)
AM_PROG_AR

AC_HEADER_SYS_WAIT
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_STRUCT_TM
AC_CANONICAL_HOST

WARN_CFLAGS=""
DEBUG_CFLAGS=""
ADD_LIB=""

use_kmem=no
require_superuser=no

enableval=""
masq_support=yes
AC_ARG_ENABLE(masq,
[  --disable-masq          disable NAT/IP masquerading support])
if test "$enableval" = "no"; then
	masq_support=no
fi

enableval=""
AC_ARG_ENABLE(nat,
[  --disable-nat           disable NAT/IP masquerading support])
if test "$enableval" = "no"; then
	masq_support=no
fi

enableval=""
ipv6_support=yes
AC_ARG_ENABLE(ipv6,
[  --disable-ipv6          disable IPv6 support])
if test "$enableval" = "no"; then
	ipv6_support=no
fi

enableval=""
libnfct_support=yes
AC_ARG_ENABLE(libnfct,
[  --disable-libnfct       disable libnetfilter_conntrack support])
if test "$enableval" = "no"; then
	libnfct_support=no
fi

enableval=""
xdgbdir_support=yes
AC_ARG_ENABLE(xdgbdir,
[  --disable-xdgbdir       disable XDG Base Directory support])
if test "$enableval" = "no"; then
	xdgbdir_support=no
fi

enableval=""
debug_support=yes
AC_ARG_ENABLE(debug,
[  --disable-debug         disable debugging support])
if test "$enableval" = "no"; then
	debug_support=no
fi

enableval=""
AC_ARG_ENABLE(warn,
[  --enable-warn           enable a variety of compiler warnings])
if test "$enableval" = "yes"; then
	WARN_CFLAGS="-O3 -ggdb -fstack-protector -fstrict-aliasing -W -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wredundant-decls -Wmissing-format-attribute -Wmissing-noreturn -Wstrict-aliasing=2 -Wlong-long -Wcomment -Wunreachable-code -Wswitch-enum -Wswitch-default -Wvolatile-register-var -Wformat=2 -Winit-self -Winvalid-pch -Winline -Wpointer-arith -Wbad-function-cast -Wcast-align -Wchar-subscripts -Wunsafe-loop-optimizations -Wdisabled-optimization -Wwrite-strings -Wsign-compare -Wconversion -Wmissing-field-initializers -Wnested-externs -Wstack-protector -Wunused-macros -Wendif-labels -Wno-cast-qual"
fi

if test "$LEX" = ":" || ! $LEX --version < /dev/null > /dev/null 2>&1; then
	AC_MSG_ERROR([No lexer generator found; please install flex])
fi

if ! $YACC --version < /dev/null > /dev/null 2>&1; then
	AC_MSG_ERROR([No parser generator found; please install bison])
fi

AC_SUBST(WARN_CFLAGS)
AC_SUBST(DEBUG_CFLAGS)

# Don't rely on __STDC_VERSION__ as kernels may use C99 booleans
AC_CHECK_HEADERS(stdbool.h)

AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h)
AC_CHECK_HEADERS(net/if.h, [], [], [
	#ifdef HAVE_SYS_SOCKET_H
		#include <sys/socket.h>
	#endif
	#ifdef HAVE_NETINET_IN_H
		#include <netinet/in.h>
	#endif

])
AC_CHECK_HEADERS(netinet/ip_compat.h netinet6/in6_pcb.h, [], [], [
	#ifdef HAVE_SYS_TYPES_H
		#include <sys/types.h>
	#endif
	#ifdef HAVE_SYS_SOCKET_H
		#include <sys/socket.h>
	#endif
	#ifdef HAVE_NETINET_IN_H
		#include <netinet/in.h>
	#endif
	#ifdef HAVE_NET_IF_H
		#include <net/if.h>
	#endif
])
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h)

AC_CHECK_TYPE(u_int32_t, uint32_t)
if test "$ac_cv_type_u_int32_t" = "no"; then
	AC_CHECK_TYPE(uint32_t, unsigned int)
fi

AC_CHECK_TYPE(u_int16_t, uint16_t)
if test "$ac_cv_type_u_int16_t" = "no"; then
	AC_CHECK_TYPE(uint16_t, unsigned short)
fi

AC_CHECK_TYPE(u_int8_t, uint8_t)
if test "$ac_cv_type_u_int8_t" = "no"; then
	AC_CHECK_TYPE(uint8_t, unsigned char)
fi

AC_MSG_CHECKING(for AF_INET6)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <netdb.h>
				#include <netinet/in_systm.h>
				#include <netinet/in.h>],
					[AF_INET6+1;])],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(AF_INET6, 10, [Define if AF_INET6 is missing])
			AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for socklen_t)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>],
					[sizeof(socklen_t);])],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(socklen_t, int, [Define to int if socklen_t is missing])
			AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for in_addr_t)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <netdb.h>
				#include <netinet/in_systm.h>
				#include <netinet/in.h>],
					[sizeof(in_addr_t);])],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(in_addr_t, unsigned int, [Define to unsigned int if in_addr_t is missing])
			AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for in_port_t)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <netdb.h>
				#include <netinet/in_systm.h>
				#include <netinet/in.h>],
					[sizeof(in_port_t);])],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(in_port_t, unsigned short, [Define to unsigned short if in_port_t is missing])
			AC_MSG_RESULT(no)])

AC_CHECK_SIZEOF(void *)
AC_DEFINE_UNQUOTED(SIZEOF_VOID_P, $ac_cv_sizeof_void_p, [Size of the 'void *' type])

AC_MSG_CHECKING(for __attribute__ unused)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <stdlib.h>],
			[int x __attribute__((unused));])],
			[AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE___ATTRIBUTE__UNUSED, 1, [Define if your compiler has __attribute__ unused])],
			[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for __attribute__ noreturn)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <stdlib.h>
static void foo(void) __attribute__((noreturn));
static void foo(void) {exit(0);}],
			[exit(0);])],
			[AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE___ATTRIBUTE__NORETURN, 1, [Define if your compiler has __attribute__ noreturn])],
			[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for __attribute__ format)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <stdlib.h>
				#include <stdarg.h>
static void foo(char *fmt, ...) __attribute__((format (printf, 1, 2)));
static void foo(char *fmt, ...) {}],
			[exit(0);])],
			[AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE___ATTRIBUTE__FORMAT, 1, [Define if your compiler has __attribute__ format])],
			[AC_MSG_RESULT(no)])

AC_CHECK_FUNCS(strsep)
AC_CHECK_FUNCS(clock_gettime select strdup strstr strtol uname)
AC_CHECK_FUNCS(arc4random_uniform lrand48 random)
AC_CHECK_FUNCS(memcmp strftime vprintf gethostbyname)
AC_CHECK_FUNCS(vasprintf asprintf)
AC_CHECK_FUNCS(inet_aton getpagesize getopt_long)
AC_CHECK_FUNCS(setgroups)
AC_CHECK_FUNCS(unveil)

AC_SEARCH_LIBS(socket, socket, , [AC_CHECK_LIB(socket, socket, LIBS="$LIBS -lsocket -lnsl", , -lsocket)])

AC_CHECK_FUNC(getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO, 1, [Define to 1 if you have the 'getaddrinfo' function.]),
	AC_SEARCH_LIBS(getaddrinfo, socket, AC_DEFINE(HAVE_GETADDRINFO, 1, [Define to 1 if you have the 'getaddrinfo' function.])))

AC_CHECK_FUNC(freeaddrinfo, AC_DEFINE(HAVE_FREEADDRINFO, 1, [Define to 1 if you have the 'freeaddrinfo' function.]),
	AC_SEARCH_LIBS(freeaddrinfo, socket, AC_DEFINE(HAVE_FREEADDRINFO, 1, [Define to 1 if you have the 'freeaddrinfo' function.])))

AC_CHECK_FUNC(gai_strerror, AC_DEFINE(HAVE_GAI_STRERROR, 1, [Define to 1 if you have the 'gai_strerror' function.]),
	AC_SEARCH_LIBS(gai_strerror, socket, AC_DEFINE(HAVE_GAI_STRERROR, 1, [Define to 1 if you have the 'gai_strerror' function.])))

AC_CHECK_FUNC(getnameinfo, AC_DEFINE(HAVE_GETNAMEINFO, 1, [Define to 1 if you have the 'getnameinfo' function.]),
	AC_SEARCH_LIBS(gai_strerror, socket, AC_DEFINE(HAVE_GETNAMEINFO, 1, [Define to 1 if you have the 'getnameinfo' function.])))

AC_CHECK_FUNC(inet_ntop, AC_DEFINE(HAVE_INET_NTOP, 1, [Define to 1 if you have the 'inet_ntop' function.]),
	AC_SEARCH_LIBS(inet_ntop, nsl, AC_DEFINE(HAVE_INET_NTOP, 1, [Define to 1 if you have the 'inet_ntop' function.])))

AC_MSG_CHECKING(for struct sockaddr_storage)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
					#include <sys/socket.h>],
				[ struct sockaddr_storage ss; ])],
				[ have_sockaddr_storage=yes ],
				[ have_sockaddr_storage=no ])
AC_MSG_RESULT($have_sockaddr_storage)

if test "$have_sockaddr_storage" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE, 1, [Define to 1 if you have struct sockaddr_storage])

	AC_MSG_CHECKING(for ss_family member in struct sockaddr_storage)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
						#include <sys/socket.h>],
					[ size_t test =
						sizeof(((struct sockaddr_storage *)0)->ss_family);])],
					[ have_ss_family=yes ],
					[ have_ss_family=no ])
	AC_MSG_RESULT($have_ss_family)

	if test "$have_ss_family" = "no"; then
		AC_MSG_CHECKING(for __ss_family member in struct sockaddr_storage)
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
							#include <sys/socket.h>],
						[ size_t test =
							sizeof(((struct sockaddr_storage *)0)->__ss_family);])],
						[ have_ss_family=yes ],
						[ have_ss_family=no ])
		AC_MSG_RESULT($have_ss_family)

		if test "$have_ss_family" = "yes"; then
			AC_DEFINE(HAVE___SS_FAMILY, 1, [Define if the sockaddr storage struct has the ss_family member])
		fi
	fi
fi

AC_MSG_CHECKING(for struct sockaddr_in6)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
					#include <netinet/in.h>],
				[ struct sockaddr_in6 s; ])],
				[ have_sockaddr_in6=yes ],
				[ have_sockaddr_in6=no ])
AC_MSG_RESULT($have_sockaddr_in6)

if test "$have_sockaddr_in6" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_IN6, 1, [Define to 1 if you have struct sockaddr_in6])
fi

AC_MSG_CHECKING(for struct in6_addr)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
					#include <netinet/in.h>],
				[ struct in6_addr s; ])],
				[ have_in6_addr=yes ],
				[ have_in6_addr=no ])
AC_MSG_RESULT($have_in6_addr)

if test "$have_in6_addr" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_IN6_ADDR, 1, [Define to 1 if you have struct in6_addr])
fi

AC_MSG_CHECKING(for struct addrinfo)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
					#include <sys/socket.h>
					#include <netdb.h>],
				[ size_t test =
					sizeof(((struct addrinfo *)0)->ai_flags);])],
				[ have_struct_addrinfo=yes ],
				[ have_struct_addrinfo=no ])
AC_MSG_RESULT($have_struct_addrinfo)

if test "$have_struct_addrinfo" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_ADDRINFO, 1, [Define to 1 if you have struct addrinfo])
fi


have_ipnat_h=""
AC_CHECK_HEADER(netinet/ip_compat.h netinet/ip_fil.h netinet/ip_nat.h)
AC_CHECK_HEADER(netinet/ip_nat.h, have_ipnat_h="yes", have_ipnat_h="no", [
	#ifdef HAVE_SYS_TYPES_H
		#include <sys/types.h>
	#endif
	#ifdef HAVE_SYS_SOCKET_H
		#include <sys/socket.h>
	#endif
	#ifdef HAVE_NET_IF_H
		#include <net/if.h>
	#endif
	#ifdef HAVE_NETINET_IN_H
		#include <netinet/in.h>
	#endif
	#ifdef HAVE_NETINET_IP_COMPAT_H
		#include <netinet/ip_compat.h>
	#endif
	#ifdef HAVE_NETINET_IP_FIL_H
		#include <netinet/ip_fil.h>
	#endif
])
if test "$have_ipnat_h" = "no"; then
	AC_CHECK_HEADER(/usr/src/sys/contrib/ipfilter/netinet/ip_nat.h, have_ipnat_h="yes", have_ipnat_h="no", [
		#ifdef HAVE_SYS_TYPES_H
			#include <sys/types.h>
		#endif
		#ifdef HAVE_SYS_SOCKET_H
			#include <sys/socket.h>
		#endif
		#ifdef HAVE_NET_IF_H
			#include <net/if.h>
		#endif
		#ifdef HAVE_NETINET_IN_H
			#include <netinet/in.h>
		#endif
		#ifdef HAVE_NETINET_IP_COMPAT_H
			#include <netinet/ip_compat.h>
		#endif
		#ifdef HAVE_NETINET_IP_FIL_H
			#include <netinet/ip_fil.h>
		#endif
	])
	if test "$have_ipnat_h" = "yes"; then
		CPPFLAGS="$CPPFLAGS -I/usr/src/sys/contrib/ipfilter"
	fi
fi

if test "$masq_support" = "yes"; then
	old_CPPFLAGS=$CPPFLAGS

	AC_MSG_CHECKING(if nat_t has nat_p member)
	AC_CACHE_VAL(ac_cv_ipf_nat_p,
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
			[
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <sys/socketvar.h>
				#include <netinet/in.h>
				#include <netinet/in_systm.h>
				#include <net/if.h>
				#include <net/route.h>
				#include <netinet/ip.h>
				#include <netinet/in_pcb.h>
				#include <arpa/inet.h>
				#include <netinet/tcp.h>

				#if defined(__OpenBSD__)
				#	include <netinet/ip_fil_compat.h>
				#else
				#	include <netinet/ip_compat.h>
				#endif

				#include <netinet/ip_fil.h>
				#include <netinet/ip_nat.h>
			],
			[int i = sizeof(((nat_t *)0)->nat_p);])],
			ac_cv_ipf_nat_p=yes,
			ac_cv_ipf_nat_p=no))
	AC_MSG_RESULT($ac_cv_ipf_nat_p)
	CPPFLAGS=$old_CPPFLAGS
fi

if test "$masq_support" = "no"; then
	libnfct_support=no
fi

want_libnfct=no

case "$host_os" in

	*linux* )
		os_src=linux.c

		if test "$masq_support" = "yes"; then
			want_libnfct=yes
		fi

		if test "$libnfct_support" = "yes"; then
			AC_CHECK_LIB(netfilter_conntrack, nfct_query, ,
					[libnfct_support=no])
		fi
	;;

	*netbsd* )
		case "$host_os" in
			*netbsd[[1-4]].* )
				AC_MSG_ERROR([cannot build on $host_os; oidentd requires NetBSD 5.0 or newer])
			;;

			* )
				use_kmem=no
				require_superuser=yes
				os_src=netbsd5.c
				masq_support=no
			;;
		esac
	;;

	*openbsd* )
		case "$host_os" in
			*openbsd[[1-2]].* )
				AC_MSG_ERROR([cannot build on $host_os; oidentd requires OpenBSD 3.0 or newer])
			;;
		esac

		os_src=openbsd30.c
	;;

	*freebsd* )
		case "$host_os" in
			*freebsd[[1-4]].* )
				AC_MSG_ERROR([cannot build on $host_os; oidentd requires FreeBSD 5.0 or newer])
			;;

			* )
				use_kmem=no
				require_superuser=yes
				os_src=freebsd5.c
				masq_support=no
			;;
		esac
	;;

	*dragonfly* )
		use_kmem=no
		require_superuser=yes
		os_src=dflybsd1.c
		masq_support=no
	;;

	* )
		AC_MSG_ERROR([oidentd does not yet support $host_os])
	;;
esac

AC_DEFINE_UNQUOTED(KERNEL_DRIVER, "$os_src", [The name of the detected kernel driver])

if test "$use_kmem" = "yes"; then
	AC_DEFINE(USE_KMEM, 1, [Set if using /dev/kmem])
	ADD_LIB="$ADD_LIB -lkvm"
else
	AC_DEFINE(USE_KMEM, 0, [Set if using /dev/kmem])
fi

if test "$require_superuser" = "yes"; then
	AC_DEFINE(NEED_ROOT, 1, [Set if privileges cannot be dropped])
	AC_MSG_WARN(privileges will not be dropped automatically on this system)
else
	AC_DEFINE(NEED_ROOT, 0, [Set if privileges cannot be dropped])
fi

if test "$masq_support" = "yes"; then
	AC_DEFINE(MASQ_SUPPORT, 1, [Set to include NAT/IP masquerading support])
else
	AC_DEFINE(MASQ_SUPPORT, 0, [Set to include NAT/IP masquerading support])
	AC_MSG_WARN(not including NAT/IP masquerade support)
fi

if test "$ipv6_support" = "yes"; then
	AC_DEFINE(WANT_IPV6, 1, [Set to include IPv6 support])
else
	AC_DEFINE(WANT_IPV6, 0, [Set to include IPv6 support])
	AC_MSG_WARN(not including IPv6 support)
fi

if test "$want_libnfct" = "yes"; then
	if test "$libnfct_support" = "yes"; then
		AC_DEFINE(LIBNFCT_SUPPORT, 1, [Set to include libnetfilter_conntrack support])
	else
		AC_MSG_WARN([You are compiling oidentd without libnetfilter_conntrack])
		AC_MSG_WARN([support.  Depending on your kernel configuration, this may])
		AC_MSG_WARN([make NAT support unavailable.  Future releases of oidentd])
		AC_MSG_WARN([may require libnetfilter_conntrack because parsing])
		AC_MSG_WARN([connection tracking files relies on an obsolete Linux kernel])
		AC_MSG_WARN([option.  Consult the INSTALL file for more information.])
	fi
fi

if test "$xdgbdir_support" = "yes"; then
	AC_DEFINE(XDGBDIR_SUPPORT, 1, [Set to include XDG Base Directory support])
else
	AC_DEFINE(XDGBDIR_SUPPORT, 0, [Set to include XDG Base Directory support])
fi

if test "$debug_support" = "yes"; then
	AC_DEFINE(ENABLE_DEBUGGING, 1, [Set to enable debugging])
else
	AC_DEFINE(ENABLE_DEBUGGING, 0, [Set to enable debugging])
fi

if test -n "$DEBUG_CFLAGS"; then
	CFLAGS="$(echo $CFLAGS | sed 's/-O[[1-9]]*/-O0/g' | sed 's/-g[[0-9]]*/-g3 -ggdb/g')"
fi

AC_SUBST(LIBS)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(ADD_LIB)
AC_SUBST(CPPFLAGS)

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/missing/Makefile
	doc/Makefile
])
AC_OUTPUT

cp -f $srcdir/src/kernel/$os_src src/os.c
