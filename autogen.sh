#!/bin/sh

AUTOCONF=autoconf
AUTOHEADER=autoheader
AUTOMAKE=automake
ACLOCAL=aclocal

($AUTOCONF --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "Error: You must have 'autoconf' installed to compile this program."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf"
	exit -1
}

($ACLOCAL --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "Error: Missing 'aclocal'. The version of \`automake'"
	echo "installed doesn't appear recent enough."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/automake"
	exit -1
}

($ACLOCAL && $AUTOHEADER && $AUTOMAKE --gnu --add-missing --copy && $AUTOCONF) || {
	echo
	echo "Error: Automatic generation of the configuration scripts has failed."
	echo "Please try to generate them manually. If you believe this faulure"
	echo "is the result of a bug in oidentd, please email ryan@numb.org with"
	echo "any relevant details."
	exit -1
}

echo "The configuration scripts have been generated successfully."
