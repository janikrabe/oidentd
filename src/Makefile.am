SUBDIRS = missing
sbin_PROGRAMS = oidentd

oidentd_LDADD = -Lmissing -lmissing $(ADD_LIB)

AM_CFLAGS = $(DEBUG_CFLAGS) $(WARN_CFLAGS)
AM_CPPFLAGS = "-I$(srcdir)/missing"

EXTRA_DIST = kernel

DISTCLEANFILES = \
	os.c

oidentd_SOURCES = \
	oidentd.c		\
	oidentd_util.c		\
	oidentd_inet_util.c	\
	oidentd_user_db.c	\
	oidentd_options.c	\
	oidentd_masq.c		\
	oidentd_cfg_scan.l	\
	oidentd_cfg_parse.y	\
	os.c

noinst_HEADERS = \
	oidentd_cfg_parse.h	\
	oidentd.h		\
	oidentd_inet_util.h	\
	oidentd_masq.h		\
	oidentd_netlink.h	\
	oidentd_options.h	\
	oidentd_user_db.h	\
	oidentd_util.h
