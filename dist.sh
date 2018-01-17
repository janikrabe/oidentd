#!/bin/sh

./scrub.sh
./autogen.sh

rm -rf .git .gitignore .travis.yml scrub.sh autogen.sh dist.sh autom4te.cache
