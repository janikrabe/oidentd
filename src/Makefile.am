SUBDIRS = missing
sbin_PROGRAMS = oidentd

LDADD =-Lmissing -lmissing

DISTCLEANFILES = \
	stamp_parse \
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

EXTRA_DIST = \
	oidentd_scan.c \
	oidentd_cfg_parse.c \
	oidentd_cfg_parse.h

YFLAGS=-d

oidentd_cfg_scan.c: oidentd_cfg_scan.l oidentd_cfg_parse.h
	@rm -f $@
	$(LEX) -ooidentd_cfg_scan.c $(srcdir)/oidentd_cfg_scan.l

stamp_parse: oidentd_cfg_parse.y
	$(YACC) $(YFLAGS) $(srcdir)/oidentd_cfg_parse.y
	touch $@

oidentd_cfg_parse.c: stamp_parse
	test -f y.tab.c && mv -f y.tab.c oidentd_cfg_parse.c || touch $@

oidentd_cfg_parse.h: stamp_parse
	test -f y.tab.h && mv -f y.tab.h oidentd_cfg_parse.h || touch $@
