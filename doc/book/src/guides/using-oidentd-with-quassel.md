<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Using oidentd with Quassel

You can use oidentd to spoof Ident replies so that they match Idents configured
within Quassel.

## Forwarding to Quassel

oidentd can forward incoming queries for connections owned by the Quassel user
directly to Quassel's built-in Ident server.
This is the recommended method.

Append the following to your [system-wide configuration file][sys-conf]:

```
user "<username>" {
	default {
		allow spoof

		# Add the next line if Quassel needs to
		# reply with the name of a local user.
		allow spoof_all

		force forward <host> <port>
	}
}
```

Replace `<username>` with the user name of the Quassel user (e.g.,
`quasselcore`).

Replace `<host>` with the <abbr title="Internet Protocol">IP</abbr> address or
hostname Quassel's built-in Ident server is configured to listen on (e.g.,
`::1`).

Replace `<port>` with the port Quassel's built-in Ident server is configured to
listen on (e.g., `10113`).

## Using Quassel's oidentd Configuration Generator

Quassel can automatically write to an oidentd user configuration file when
establishing a new connection.

Use of this feature is discouraged as of oidentd 2.3.0.
Some <abbr title="Internet Relay Chat">IRC</abbr> servers send Ident queries
before the connection's local port is known to Quassel, which may cause lookups
to fail.

Append the following to your [system-wide configuration file][sys-conf]:

```
user "<username>" {
	default {
		allow spoof

		# Use this only if Quassel needs to spoof local user names
		allow spoof_all
	}
}
```

Replace `<username>` with the user name of the Quassel user (e.g.,
`quasselcore`).

Ensure that the Quassel home directory (typically `~quasselcore`) is
world-executable (mode `0711`) so that oidentd can read the user configuration
file:

```
chmod 0711 ~quasselcore
```

Finally, make sure Quassel is run with the `--oidentd` flag.

Your changes will take effect after you reload oidentd and restart Quassel.

[sys-conf]: ../getting-started/configuration/index.md#system-wide-configuration-file
