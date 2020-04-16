---
# Copyright (c)  2018-2020  Janik Rabe
#
# Permission is granted to copy, distribute and/or modify this document
# under the terms of the GNU Free Documentation License, Version 1.3
# or any later version published by the Free Software Foundation;
# with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
# A copy of the license is included in the file 'COPYING.DOC'

title: "Identification vs. Authentication"
weight: 3
---

The Ident protocol was designed for identification, not authentication.
Please don't use it for access control.

The primary purpose of the Ident protocol is to serve as an auditing and abuse
prevention mechanism.
For example, many {{< abbr "IRC" "Internet Relay Chat" >}} servers act as Ident
clients, querying and publicly displaying users' Ident replies.
This allows providers of {{< abbr "IRC" "Internet Relay Chat" >}} bouncers,
shell accounts and other services to identify users abusing their systems and
enables channel operators and network operators to remove individual users
without excluding their entire host or network.

Ident queries and replies are sent as plain text, with no encryption or
authentication, and can be intercepted or modified by an attacker.
In addition, it is not possible to prevent compromised or malicious hosts from
[sending arbitrary Ident replies][cap-spoof].
For these reasons, the Ident protocol is not suitable for authentication or
access control.

[cap-spoof]: {{% ref "../getting-started/capabilities#spoof" %}}
