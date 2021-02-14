<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Dropping Privileges

It is highly recommended not to run internet-facing services as root.
For this reason, oidentd attempts to switch to an unprivileged user
automatically after starting up.

Please note that oidentd needs to run as root on a small number of systems.
On these systems, a warning is printed at startup and privileges are not
dropped automatically.
Your system is affected by this limitation if `oidentd --version` prints "Needs
root access: Yes".

## Default User

By default, oidentd attempts to run as one of the following users, in order of
preference.
If a user does not exist, oidentd tries to use the next one.

* oidentd
* nobody

If neither of the above users exists, oidentd switches to user ID 65534.

## Default Group

By default, oidentd attempts to run as one of the following groups, in order of
preference.
If a group does not exist, oidentd tries to use the next one.

* oidentd
* nobody
* nogroup

If none of the above groups exist, oidentd switches to group ID 65534.

## Changing This Behavior

The `--user` and `--group` options can be used to run oidentd as another user
or group.
oidentd refuses to start up if the user or group specified by either of these
options does not exist, or if privileges cannot be dropped for some other
reason.
