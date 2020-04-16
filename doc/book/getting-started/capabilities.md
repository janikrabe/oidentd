---
# Copyright (c)  2018-2020  Janik Rabe
#
# Permission is granted to copy, distribute and/or modify this document
# under the terms of the GNU Free Documentation License, Version 1.3
# or any later version published by the Free Software Foundation;
# with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
# A copy of the license is included in the file 'COPYING.DOC'

title: "Capabilities"
weight: 2
---

Capabilities allow the system administrator to control the set of features
users have access to.
They can be granted or revoked using [capability directives][capdirs].
No capabilities are granted to users by default.

## `forward`

The `forward` capability allows users to forward Ident queries to another
server.
The `host` and `port` arguments specify the host and port of the receiving
Ident server.
This server must support forwarding (e.g., oidentd with the `--proxy` option).

Forwarding does not allow users to send replies they otherwise would not be
able to send.
For example, if the receiving Ident server replies with the name of another
user, the reply will be sent back to the client only if the user that owns the
connection was granted the `spoof` and `spoof_all` capabilities.
This restriction does not apply to `force forward` statements in the
system-wide configuration file.

Imperative syntax: `forward <host> <port>`

## `hide`

The `hide` capability allows users to hide their connections.
When used, oidentd reports a `HIDDEN-USER` error to clients.

Imperative syntax: `hide`

## `numeric`

The `numeric` capability allows users to reply with their user ID (UID) instead
of their user name.

Imperative syntax: `numeric`

## `random`

The `random` capability allows users to send random alphanumeric Ident replies.
Replies are logged so that the system administrator can identify the user
responsible for a particular connection.

Imperative syntax: `random`

## `random_numeric`

The `random_numeric` capability allows users to send random numeric Ident
replies of the form `userNNNNN`, where `N` represents a digit from 0 to 9.
Replies are logged so that the system administrator can identify the user
responsible for a particular connection.

Imperative syntax: `random_numeric`

## `reply`

The `reply` capability cannot be granted or revoked.
However, using it may require one or more of `spoof`, `spoof_all`, and
`spoof_privport`, depending on the replies and type of connection.

If more than one reply is given, a random reply is chosen from the list for
each incoming query.
At least one reply must be specified.

Imperative syntax: `reply <replies ...>`

## `spoof`

The `spoof` capability allows users to send custom Ident replies, except in
cases that require the `spoof_all` or `spoof_privport` capabilities.

This capability does not have an imperative syntax.

## `spoof_all`

The `spoof_all` capability allows users to reply with the names of other users
on the system.
It only works in conjunction with the `spoof` capability.

This capability does not have an imperative syntax.

## `spoof_privport`

The `spoof_privport` capability allows users to spoof replies for connections
to privileged foreign ports (port numbers below 1024).
It only works in conjunction with the `spoof` capability.

This capability does not have an imperative syntax.

[capdirs]: {{% ref "configuration#capability-directives" %}}
