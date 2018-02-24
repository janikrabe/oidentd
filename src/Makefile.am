SUBDIRS = missing
sbin_PROGRAMS = oidentd

oidentd_LDADD = -Lmissing -lmissing $(ADD_LIB)

AM_CFLAGS = $(DEBUG_CFLAGS) $(WARN_CFLAGS)
AM_CPPFLAGS = -I "$(srcdir)/missing" \
	-D "SYSCONFDIR=\"$(sysconfdir)\""

EXTRA_DIST = kernel

DISTCLEANFILES = \
	os.c

oidentd_SOURCES = \
	oidentd.c	\
	util.c		\
	inet_util.c	\
	user_db.c	\
	options.c	\
	masq.c		\
	cfg_scan.l	\
	cfg_parse.y	\
	os.c

noinst_HEADERS = \
	oidentd.h	\
	cfg_parse.h	\
	inet_util.h	\
	masq.h		\
	netlink.h	\
	options.h	\
	user_db.h	\
	util.h
