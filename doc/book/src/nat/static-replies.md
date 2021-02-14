<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Static Replies

oidentd can send different Ident replies for each host connecting through the
machine it is running on.

To enable this functionality, oidentd must be run with the `--masquerade`
(`-m`) flag.

## Configuring Replies

Replies can be configured in the masquerading configuration file, which is
usually found in `/etc/oidentd_masq.conf` or in
`/usr/local/etc/oidentd_masq.conf`, depending on how oidentd was installed.

This file should consist of lines of the following form:

```
<host>[/subnet] ident_reply system_type
```

The masquerading configuration file is read from top to bottom, so more
specific rules should be placed before more general ones.
For example:

```
10.0.0.1     user1  UNIX
10.0.0.2     user2  FREEBSD
10.0.0.0/24  user3  OTHER
```

In this example, a `user1` reply is sent in response to all requests for
`10.0.0.1`.
A `user2` reply is sent for all requests for `10.0.0.2`.
Finally, `user3` is sent in response to requests for other machines in the
`10.0.0.0/24` subnetwork.

Detailed information can be found in the `oidentd_masq.conf(5)` manual page.
