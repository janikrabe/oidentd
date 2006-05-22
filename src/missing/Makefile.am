INCLUDES = -I../../src
AM_CFLAGS = $(DEBUG_CFLAGS) $(WARN_CFLAGS)

noinst_LIBRARIES = libmissing.a

libmissing_a_SOURCES =  \
	inet_aton.c			\
	inet_ntop.c			\
	ipv6_missing.c		\
	getopt.c			\
	snprintf.c			\
	vasprintf.c

noinst_HEADERS = \
	getopt_missing.h	\
	oidentd_missing.h
