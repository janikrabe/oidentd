#!/bin/sh

./scrub.sh
./autogen.sh
find . \( -name CVS -or -name .cvsignore \) -exec rm -rf {} \; >& /dev/null
rm -rf scrub.sh autogen.sh dist.sh autom4te.cache
