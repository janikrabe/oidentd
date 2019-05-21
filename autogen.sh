#!/usr/bin/env sh

AUTOCONF=autoconf
AUTOMAKE=automake
AUTOHEADER=autoheader
ACLOCAL=aclocal

require_binary() {
	"$1" --version < /dev/null > /dev/null 2>&1 || {
		echo "Error: No usable installation of '$1' was found in your \$PATH."
		echo "       Please install it to compile oidentd."

		test -z "$2" || {
			echo
			echo "Note: $2"
		}

		exit 1
	}
}

require_binary "$AUTOCONF"
require_binary "$AUTOMAKE"
require_binary "$AUTOHEADER" \
	"Your version of 'automake' may not be recent enough."
require_binary "$ACLOCAL" \
	"Your version of 'automake' may not be recent enough."

$ACLOCAL && $AUTOHEADER && $AUTOMAKE --gnu --add-missing --copy && $AUTOCONF || {
	echo "Error: Automatic generation of the configuration scripts has failed."
	echo "       Please try to generate them manually.  If you believe this"
	echo "       failure is the result of a bug in oidentd, please email"
	echo "       <oidentd@janikrabe.com> with any relevant details."
	exit 1
}
