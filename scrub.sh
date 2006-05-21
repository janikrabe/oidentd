#!/bin/sh

find . \( -name .deps -or -name Makefile -or -name Makefile.in -or -name \*.o -or -name \*.a \) -exec /bin/rm -rf {} \; 2>/dev/null

/bin/rm -rf src/oidentd stamp-h1 mkinstalldirs missing install-sh depcomp configure config.sub config.status config.log config.h.in~ config.h.in config.h config.guess autom4te.cache aclocal.m4
