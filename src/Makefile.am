SUBDIRS = missing
sbin_PROGRAMS = oidentd

LDADD =-Lmissing -lmissing

DISTCLEANFILES = \
	os.c

oidentd_SOURCES = \
	oidentd.c \
	oidentd_util.c \
	oidentd_inet_util.c \
	oidentd_user_db.c \
	oidentd_options.c \
	oidentd_masq.c \
	oidentd_cfg_scan.l \
	oidentd_cfg_parse.y \
	os.c
