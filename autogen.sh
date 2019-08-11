#!/usr/bin/env sh

# autogen.sh - oidentd ident (RFC 1413) implementation.
# Copyright (c) 2019 Janik Rabe  <oidentd@janikrabe.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
