<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Examples

The following examples illustrate how capabilities and conditional directives
can be used in system-wide and user configuration files.

Lines stating with `#` are comments.
They are ignored by oidentd.

## System-wide Configuration File

This configuration allows `ryan` to send spoofed and random Ident replies,
except in response to lookups for connections to `example.net`.
Other users' connections are hidden so that no user information is disclosed.

```
default {  # User defaults
	default {  # Connection defaults
		# Hide all connections from users not
		# explicitly listed in this file.
		force hide
	}
}

user ryan {  # Settings for user "ryan"

	default {  # Connection defaults
		# Allow ryan to send custom Ident replies,
		# except for the names of other users.
		allow spoof

		# Allow ryan to send random Ident replies.
		allow random
	}

	to example.net {  # Connections to example.net
		# Do not allow ryan to spoof Ident replies.
		force reply "ryan"
	}
}
```

## User Configuration File

This user configuration file instructs oidentd to reply to Ident queries for
connections to foreign ports `6667` through `6697` with the name of a random
Greek letter.
This requires the `spoof` capability.
It also requires the `spoof_all` capability if there is a local user named
"alpha", "beta", or "gamma".

A random alphanumeric Ident reply is sent in response to all other queries.
This requires the `random` capability.

```
global {  # Connection defaults
	# Send random Ident replies by default.
	random
}

fport 6667:6697 {  # Foreign ports from 6667 to 6697
	# Choose one of three Greek letters at random.
	reply "alpha" "beta" "gamma"
}
```
