dnl Copyright (C) 2006 Ryan McCabe <ryan@numb.org> (GPL v2)

AC_INIT(oidentd, 2.0.8, ryan@numb.org)
AM_INIT_AUTOMAKE(oidentd, 2.0.8)

AC_DEFINE(PACKAGE_WEBSITE, ["http://dev.ojnk.net"], [website])
AC_DEFINE(PACKAGE_AUTHOR, ["Ryan McCabe"], [author])

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AM_PROG_LEX
AC_PATH_PROG(LEX, flex)
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PATH_PROG(sedpath, sed)

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_CANONICAL_HOST

WARN_CFLAGS=""
DEBUG_CFLAGS=""
ADD_LIB=""

use_kmem=no

masq_support=yes
AC_ARG_ENABLE(masq, [  --disable-masq          Disable NAT/IP masquerading support])
if test "$enableval" = "no"; then
	masq_support=no
fi

masq_support=yes
AC_ARG_ENABLE(nat, [  --disable-nat           Disable NAT/IP masquerading support])
if test "$enableval" = "no"; then
	masq_support=no
fi

enableval=""
ipv6_support=yes
AC_ARG_ENABLE(ipv6, [  --disable-ipv6          Disable IPv6 support])
if test "$enableval" = "no"; then
	ipv6_support=no
fi


enableval=""
AC_ARG_ENABLE(debug,
[  --enable-debug          Enable debugging])
if test "$enableval" = "yes"; then
	AC_DEFINE(ENABLE_DEBUGGING, 1, [Define to enable debugging])
	DEBUG_CFLAGS="-O0 -fvar-tracking -g3 -ggdb"
fi

enableval=""
AC_ARG_ENABLE(warn,
[  --enable-warn		  Enable a bunch of compiler warnings])
if test "$enableval" = "yes"; then
	WARN_CFLAGS="-O3 -ggdb -fstack-protector -fstrict-aliasing -W -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wredundant-decls -Wmissing-format-attribute -Wmissing-noreturn -Waggregate-return -Wstrict-aliasing=2 -Wlong-long -Wcomment -Wunreachable-code -Wswitch-enum -Wswitch-default -Wvolatile-register-var -Wformat=2 -Winit-self -Winvalid-pch -Winline -Wpointer-arith -Wbad-function-cast -Wcast-align -Wchar-subscripts -Wunsafe-loop-optimizations -Wdisabled-optimization -Wwrite-strings -Wsign-compare -Wconversion -Wmissing-field-initializers -Wnested-externs -Wstack-protector -Wunused-macros -Wendif-labels -Wno-cast-qual"
fi

AC_SUBST(WARN_CFLAGS)
AC_SUBST(DEBUG_CFLAGS)
AC_SUBST(FLEX_PATH)

AC_CHECK_HEADERS(netinet/ip_compat.h netinet6/in6_pcb.h)
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
AC_TRY_COMPILE([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <netdb.h>
				#include <netinet/in_systm.h>
				#include <netinet/in.h>],
					[AF_INET6+1;],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(AF_INET6, 10, [Define if AF_INET6 is missing])
			AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for socklen_t)
AC_TRY_COMPILE([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>],
					[sizeof(socklen_t);],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(socklen_t, int, [Define to int if socklen_t is missing])
			AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for in_addr_t)
AC_TRY_COMPILE([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <netdb.h>
				#include <netinet/in_systm.h>
				#include <netinet/in.h>],
					[sizeof(in_addr_t);],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(in_addr_t, unsigned int, [Define to unsigned int if in_addr_t is missing])
			AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for in_port_t)
AC_TRY_COMPILE([#include <unistd.h>
				#include <sys/types.h>
				#include <sys/socket.h>
				#include <netdb.h>
				#include <netinet/in_systm.h>
				#include <netinet/in.h>],
					[sizeof(in_port_t);],
			[AC_MSG_RESULT(yes)],
			[AC_DEFINE(in_port_t, unsigned short, [Define to unsigned short if in_port_t is missing])
			AC_MSG_RESULT(no)])

AC_CHECK_SIZEOF(void *)
AC_DEFINE_UNQUOTED(SIZEOF_VOID_P, $ac_cv_sizeof_void_p, [Size of the 'void *' type])

AC_MSG_CHECKING(for __attribute__ unused)
AC_TRY_COMPILE([#include <stdlib.h>],
			[int x __attribute__((unused));],
			[AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE___ATTRIBUTE__UNUSED, 1, [Define if your compiler has __attribute__ unused])],
			[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for __attribute__ noreturn)
AC_TRY_COMPILE([#include <stdlib.h>],
			[ exit(0); } static void foo(void) __attribute__((noreturn));
			static void foo(void) {exit(0); ],
			[AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE___ATTRIBUTE__NORETURN, 1, [Define if your compiler has __attribute__ noreturn])],
			[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for __attribute__ format)
AC_TRY_COMPILE([#include <stdlib.h>
				#include <stdarg.h>],
			[ exit(0); } static int foo(char *fmt, ...) __attribute__((format (printf, 1, 2)));
			static int foo(char *fmt, ...) { ],
			[AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE___ATTRIBUTE__FORMAT, 1, [Define if your compiler has __attribute__ format])],
			[AC_MSG_RESULT(no)])

AC_TYPE_SIGNAL

AC_CHECK_FUNCS(snprintf vsnprintf getopt_long strsep vasprintf)
AC_CHECK_FUNCS(gettimeofday select strdup strstr strtol uname)
AC_CHECK_FUNCS(memcmp strftime vprintf gethostbyname)
AC_CHECK_FUNCS(vasprintf asprintf snprintf vsnprintf)
AC_CHECK_FUNCS(inet_aton getpagesize getopt_long)

AC_SEARCH_LIBS(socket, socket, , [AC_CHECK_LIB(socket, socket, LIBS="$LIBS -lsocket -lnsl", , -lsocket)])

AC_CHECK_FUNC(getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO, 1, [Define to 1 if you have the 'getaddrinfo' function.]),
	AC_CHECK_LIB(socket, getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO, 1, [Define to 1 if you have the 'getaddrinfo' function.])))

AC_CHECK_FUNC(freeaddrinfo, AC_DEFINE(HAVE_FREEADDRINFO, 1, [Define to 1 if you have the 'freeaddrinfo' function.]),
	AC_CHECK_LIB(socket, freeaddrinfo), AC_DEFINE(HAVE_FREEADDRINFO, 1, [Define to 1 if you have the 'freeaddrinfo' function.]))

AC_CHECK_FUNC(gai_strerror, AC_DEFINE(HAVE_GAI_STRERROR, 1, [Define to 1 if you have the 'gai_strerror' function.]),
	AC_CHECK_LIB(socket, gai_strerror, AC_DEFINE(HAVE_GAI_STRERROR, 1, [Define to 1 if you have the 'gai_strerror' function.])))

AC_CHECK_FUNC(getnameinfo, AC_DEFINE(HAVE_GETNAMEINFO, 1, [Define to 1 if you have the 'getnameinfo' function.]),
	AC_CHECK_LIB(socket, gai_strerror, AC_DEFINE(HAVE_GETNAMEINFO, 1, [Define to 1 if you have the 'getnameinfo' function.])))

AC_CHECK_FUNC(inet_ntop, AC_DEFINE(HAVE_INET_NTOP, 1, [Define to 1 if you have the 'inet_ntop' function.]),
	AC_CHECK_LIB(nsl, inet_ntop, AC_DEFINE(HAVE_INET_NTOP, 1, [Define to 1 if you have the 'inet_ntop' function.])))

AC_CHECK_LIB(udb, main)

AC_MSG_CHECKING(for struct sockaddr_storage)
AC_TRY_COMPILE(	[	#include <sys/types.h>
					#include <sys/socket.h>],
				[ struct sockaddr_storage ss; ],
				[ have_sockaddr_storage=yes ],
				[ have_sockaddr_storage=no ])
AC_MSG_RESULT($have_sockaddr_storage)

if test "$have_sockaddr_storage" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE, 1, [Define to 1 if you have struct sockaddr_storage])

	AC_MSG_CHECKING(for ss_family member in struct sockaddr_storage)
	AC_TRY_COMPILE(	[	#include <sys/types.h>
						#include <sys/socket.h>],
					[ size_t test =
						sizeof(((struct sockaddr_storage *)0)->ss_family);],
					[ have_ss_family=yes ],
					[ have_ss_family=no ])
	AC_MSG_RESULT($have_ss_family)

	if test "$have_ss_family" = "no"; then
		AC_MSG_CHECKING(for __ss_family member in struct sockaddr_storage)
		AC_TRY_COMPILE(	[	#include <sys/types.h>
							#include <sys/socket.h>],
						[ size_t test =
							sizeof(((struct sockaddr_storage *)0)->__ss_family);],
						[ have_ss_family=yes ],
						[ have_ss_family=no ])
		AC_MSG_RESULT($have_ss_family)

		if test "$have_ss_family" = "yes"; then
			AC_DEFINE(HAVE___SS_FAMILY, 1, [Define if the sockaddr storage struct has the ss_family member])
		fi
	fi
fi

AC_MSG_CHECKING(for struct sockaddr_in6)
AC_TRY_COMPILE(	[	#include <sys/types.h>
					#include <netinet/in.h>],
				[ struct sockaddr_in6 s; ],
				[ have_sockaddr_in6=yes ],
				[ have_sockaddr_in6=no ])
AC_MSG_RESULT($have_sockaddr_in6)

if test "$have_sockaddr_in6" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_IN6, 1, [Define to 1 if you have struct sockaddr_in6])
fi

AC_MSG_CHECKING(for struct in6_addr)
AC_TRY_COMPILE(	[	#include <sys/types.h>
					#include <netinet/in.h>],
				[ struct in6_addr s; ],
				[ have_in6_addr=yes ],
				[ have_in6_addr=no ])
AC_MSG_RESULT($have_in6_addr)

if test "$have_in6_addr" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_IN6_ADDR, 1, [Define to 1 if you have struct in6_addr])
fi

AC_MSG_CHECKING(for struct addrinfo)
AC_TRY_COMPILE( [	#include <sys/types.h>
					#include <sys/socket.h>
					#include <netdb.h>],
				[ size_t test =
					sizeof(((struct addrinfo *)0)->ai_flags);],
				[ have_struct_addrinfo=yes ],
				[ have_struct_addrinfo=no ])
AC_MSG_RESULT($have_struct_addrinfo)

if test "$have_struct_addrinfo" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_ADDRINFO, 1, [Define to 1 if you have struct addrinfo])
fi


have_ipnat_h=""
AC_CHECK_HEADER(netinet/ip_nat.h, have_ipnat_h="yes", have_ipnat_h="no")
if test "$have_ipnat_h" = "no"; then
	AC_CHECK_HEADER(/usr/src/sys/contrib/ipfilter/netinet/ip_nat.h, have_ipnat_h="yes", have_ipnat_h="no")
	if test "$have_ipnat_h" = "yes"; then
		CPPFLAGS="$CPPFLAGS -I/usr/src/sys/contrib/ipfilter"
	fi
fi

if test "$masq_support" = "yes"; then
	old_CPPFLAGS=$CPPFLAGS

	if test `uname -s` = "SunOS"; then
		release=`uname -r | cut -d. -f2`
		CPPFLAGS="$CPPFLAGS -DSOLARIS=$release"
	fi

	AC_MSG_CHECKING(if nat_t has nat_p member)
	AC_CACHE_VAL(ac_cv_ipf_nat_p,
		AC_TRY_COMPILE(
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
				#	if defined(SOLARIS) && SOLARIS >= 8
				#		include <netinet/ip6.h>
				#	endif
				#	include <netinet/ip_compat.h>
				#endif

				#include <netinet/ip_fil.h>
				#include <netinet/ip_nat.h>
			],
			[int i = sizeof(((nat_t *)0)->nat_p);],
			ac_cv_ipf_nat_p=yes,
			ac_cv_ipf_nat_p=no))
	AC_MSG_RESULT($ac_cv_ipf_nat_p)
	CPPFLAGS=$old_CPPFLAGS
fi

case "$host_os" in

	*linux* )
		os_src=linux.c
	;;

	*netbsd* )
		use_kmem=yes
		os_src=netbsd.c
		if test "$ac_cv_ipf_nat_p" = "no"; then
			masq_support=no
		fi
	;;

	*darwin* )
		CFLAGS="-no-cpp-precomp $CFLAGS"
		use_kmem=yes
		os_src=darwin.c
		if test "$ac_cv_ipf_nat_p" = "no"; then
			masq_support=no
		fi
	;;

	*openbsd* )
		case "$host_os" in
			*openbsd2* )
				if test "$ac_cv_ipf_nat_p" = "no"; then
					masq_support=no
				fi
			;;
		esac

		if test "$masq_support" = "no"; then
			case "$host_os" in
				*openbsd2.[[0123]]* )
					use_kmem=yes
					os_src=openbsd.c
				;;

				*openbsd2.[[45678]])
					os_src=openbsd24.c
				;;

				*openbsd2.9*)
					os_src=openbsd29.c
				;;

				*openbsd3*)
					os_src=openbsd30.c
				;;
			esac
		else
			case "$host_os" in
				*openbsd2* )
					use_kmem=yes
					os_src=openbsd.c
				;;

				*openbsd3* )
					os_src=openbsd30.c
				;;
			esac
		fi
	;;

	*freebsd* )
		if test "$ac_cv_ipf_nat_p" = "no"; then
			masq_support=no
		fi

		if test "$masq_support" = "no"; then
			case "$host_os" in
				*freebsd[[4-9]]* )
					use_kmem=no
					os_src=freebsd4.c
				;;

				*)
					use_kmem=yes
					os_src=freebsd.c
				;;
			esac
		else
			case "$host_os" in
				*freebsd[[5-9]]* )
					use_kmem=yes
					os_src=freebsd5.c
				;;
				*)
					use_kmem=yes
					os_src=freebsd.c
				;;
			esac
		fi
	;;

	*solaris2.4* )
		AC_DEFINE(SOLARIS, 4, [Solaris version number])
		use_kmem=yes
		masq_support=no
		ipv6_support=no
		os_src=solaris4.c

		ADD_LIB="$ADD_LIB -lelf"
	;;

	*solaris2.5* )
		AC_DEFINE(SOLARIS, 5, [Solaris version number])
		use_kmem=yes
		masq_support=no
		ipv6_support=no
		os_src=solaris5.c

		ADD_LIB="$ADD_LIB -lelf"
	;;

	*solaris2.6* )
		AC_DEFINE(SOLARIS, 6, [Solaris version number])
		use_kmem=yes
		masq_support=no
		ipv6_support=no
		os_src=solaris7.c

		ADD_LIB="$ADD_LIB -lelf"
	;;

	*solaris2.7* )
		AC_DEFINE(SOLARIS, 7, [Solaris version number])
		use_kmem=yes
		masq_support=no
		ipv6_support=no
		os_src=solaris7.c

		ADD_LIB="$ADD_LIB -lelf"
	;;

	*solaris2.8* )
		AC_DEFINE(SOLARIS, 8, [Solaris version number])
		use_kmem=yes
		masq_support=no
		ipv6_support=no
		os_src=solaris8.c

		ADD_LIB="$ADD_LIB -lelf"
	;;

	*solaris* )
		AC_DEFINE(SOLARIS, 8, [Solaris version number])
		use_kmem=yes
		masq_support=no
		ipv6_support=no
		os_src=solaris8.c

		ADD_LIB="$ADD_LIB -lelf"
	;; 

	*)
		echo $host_os is not yet supported.
		exit 1
	;;
esac

if test "$use_kmem" = "yes"; then
	AC_DEFINE(USE_KMEM, 1, [Define if using /dev/kmem])
	ADD_LIB="$ADD_LIB -lkvm"
fi

if test "$masq_support" = "yes"; then
	AC_DEFINE(MASQ_SUPPORT, 1, [Define to include NAT/IP masq support])
else
	AC_MSG_WARN(not including NAT/IP masquerade support.)
fi

if test "$ipv6_support" = "yes"; then
	AC_DEFINE(WANT_IPV6, 1, [Define to include IPv6 support])
else
	AC_MSG_WARN(not including IPv6 support.)
fi

if test -n "$DEBUG_CFLAGS"; then
	CFLAGS="$(echo $CFLAGS | sed 's/-O[[1-9]]*/-O0/g' | sed 's/-g[[0-9]]*/-g3 -ggdb/g')"
fi

AC_SUBST(LIBS)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(ADD_LIB)
AC_SUBST(CPPFLAGS)

AC_OUTPUT([
	Makefile
	src/Makefile
	src/missing/Makefile
	doc/Makefile
])

cp -f $srcdir/src/kernel/$os_src src/os.c
