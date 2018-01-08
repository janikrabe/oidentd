#!/bin/sh

./scrub.sh
./autogen.sh

rm -rf .git .gitignore scrub.sh autogen.sh dist.sh autom4te.cache
