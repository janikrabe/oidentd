# Test suite for oidentd
# Copyright (C) 2018 Janik Rabe <info@janikrabe.com>
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA

test_srcdir="$(realpath "$test_srcdir")"
test_testdir="$(realpath "${test_srcdir}/../test")"

export test_srcdir
export test_testdir

cd "$test_testdir"

sed "s:CURRENT_USER_WHOAMI:$(whoami):" oidentd.conf.default > oidentd.conf
