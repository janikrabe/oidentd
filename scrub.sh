#!/usr/bin/env sh

find . \( -name .deps -or -name Makefile -or -name Makefile.in -or -name \*.o -or -name \*.a \) -exec /bin/rm -rf {} \; 2>/dev/null

/bin/rm -rf \
	aclocal.m4	\
	autom4te.cache	\
	configure	\
	config.sub	\
	config.status	\
	config.log	\
	config.h	\
	config.h.in	\
	config.h.in~	\
	config.guess	\
	depcomp		\
	dist		\
	install-sh	\
	mkinstalldirs	\
	missing		\
	src/oidentd	\
	src/os.c	\
	stamp-h1	\
	test-driver	\
	ylwrap
