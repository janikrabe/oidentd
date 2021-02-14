<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Using oidentd with ZNC

You can use oidentd to spoof Ident replies so that they match Idents configured
within ZNC.

Append the following to your [system-wide configuration file][sys-conf]:

```
user "<username>" {
	default {
		allow spoof

		# Add the next line if ZNC needs to
		# reply with the name of a local user.
		allow spoof_all
	}
}
```

Replace `<username>` with the user name of the ZNC user (e.g., `znc`).

Change to the user running ZNC (e.g., using `su znc -ls "$SHELL"`), and use the
following commands to create an empty [user configuration file][usr-conf]:

```
touch ~/.oidentd.conf
chmod 0644 ~/.oidentd.conf
```

Ensure that the ZNC home directory (typically `~znc`) is world-executable (mode
`0711`) so that oidentd can read the user configuration file:

```
chmod 0711 ~
```

Ensure that ZNC's *identfile* module is loaded and configured correctly:

```
/MSG *status LoadMod identfile
/MSG *identfile SetFile ~/.oidentd.conf
/MSG *identfile SetFormat global { reply "%user%" }
/MSG *status SaveConfig
```

Note that `%user%` expands to the name of the ZNC user that initiated the
connection.
Another popular choice is `%ident%`, which allows users to specify any Ident.

Your changes will take effect after you reload oidentd and reconnect to
<abbr title="Internet Relay Chat">IRC</abbr>.

[sys-conf]: ../getting-started/configuration/index.md#system-wide-configuration-file
[usr-conf]: ../getting-started/configuration/index.md#user-configuration-file
