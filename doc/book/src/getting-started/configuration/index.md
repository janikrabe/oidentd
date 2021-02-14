<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Configuration

When an Ident query is received, oidentd normally replies with the user name of
the user that owns the corresponding connection.
Users can override this behavior only if they have been granted permission to
do so through the system-wide configuration file.

## System-wide Configuration File

The system-wide configuration file is usually found at `/etc/oidentd.conf` or
`/usr/local/etc/oidentd.conf`, depending on how oidentd was installed.
It is not necessary for this file to exist.

This file may contain any number of [user directives][user-directives].

## User Configuration File

Each user may also create a user configuration file at `~/.config/oidentd.conf`
or `~/.oidentd.conf`.
If both files exist, only the former is used.
The user configuration file is ignored if oidentd does not have permission to
read it.

This file may contain one directive of the following form:

```
global {
	<capability-statements ...>
}
```

This `global` directive matches all connections.
If used, it should be the first directive in the file.

The user configuration file may also contain any number of directives of the
following form:

```
<range-specification> {
	<capability-statements ...>
}
```

In this form the directive only applies to connections that match the given
range specification.

## User Directives

A user directive takes one of the following forms:

```
default {
	<range-directives ...>
}
```

This form can be used to specify defaults for users.
There should be no more than one directive of this form.
If used, it should be the first user directive.

```
user "<username>" {
	<range-directives ...>
}
```

In this form the directive applies only to the specified user.

## Range Directives

A range directive takes one of the following forms:

```
default {
	<capability-directives ...>
}
```

In this form the directive matches when no other range directive in its scope
does.
There should be no more than one directive of this form.
If used, it should be the first range directive.

```
<range-specification> {
	<capability-directives ...>
}
```

In this form the directive only applies to connections that match the given
range specification.

## Range Specification

A range specification takes the following form:

```
[to <host>] [fport <port>] [from <host>] [lport <port>]
```

* `to` is the foreign address associated with the connection.
* `fport` is the foreign port associated with the connection.
* `from` is the local address associated with the connection.
* `lport` is the local port associated with the connection.

At least one of the four filters must be specified.

Hosts may be specified by hostname or by
<abbr title="Internet Protocol">IP</abbr> address.
Ports may optionally be specified as a port range of the form `min:max`,
`min:`, or `:max`.

A range specification matches a connection if all specified filters match.

## Capability Directives

A capability directive takes one of the following forms:

```
allow <capability>
```

In this form the directive grants the specified capability.

```
deny <capability>
```

In this form the directive revokes the specified capability.

```
force <capability-statement>
```

In this form the directive enforces use of the specified capability.

## Capabilities

The following are valid capabilities:

* [`forward`][cap-forward]
* [`hide`][cap-hide]
* [`numeric`][cap-numeric]
* [`random`][cap-random]
* [`random_numeric`][cap-random_numeric]
* [`spoof`][cap-spoof]
* [`spoof_all`][cap-spoof_all]
* [`spoof_privport`][cap-spoof_privport]

## Capability Statements

The following are valid capability statements:

* [`forward <host> <port>`][cap-forward]
* [`hide`][cap-hide]
* [`numeric`][cap-numeric]
* [`random`][cap-random]
* [`random_numeric`][cap-random_numeric]
* [`reply <replies ...>`][cap-reply]

## Further Reading

The `oidentd.conf(5)` manual page contains further information on how to
configure oidentd, as well as detailed descriptions of all capabilities.

[user-directives]:    #user-directives
[cap-forward]:        ../capabilities.md#forward
[cap-hide]:           ../capabilities.md#hide
[cap-numeric]:        ../capabilities.md#numeric
[cap-random]:         ../capabilities.md#random
[cap-random_numeric]: ../capabilities.md#random_numeric
[cap-reply]:          ../capabilities.md#reply
[cap-spoof]:          ../capabilities.md#spoof
[cap-spoof_all]:      ../capabilities.md#spoof_all
[cap-spoof_privport]: ../capabilities.md#spoof_privport
