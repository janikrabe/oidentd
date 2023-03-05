<!--
Copyright (c)  2018-2023  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# About oidentd

_Flexible, RFC 1413 compliant Ident daemon with NAT support._

oidentd is a flexible, <abbr title="Request for Comments">RFC</abbr> 1413
compliant Ident server.
It runs on Linux, FreeBSD, OpenBSD, NetBSD, and DragonFly BSD.
It is highly configurable, allowing the system administrator to define custom
responses based on host and port pairs.
The administrator can also grant capabilities to individual users to allow them
to change their Ident replies, generate random replies, or hide their
connections.
oidentd supports lookups for NAT connections and is able to forward queries to
other servers.

oidentd was originally written by Ryan McCabe in 1998. Since January 2018, it
is maintained by Janik Rabe, with contributions from several other volunteers.

The Ident Protocol is used primarily on
<abbr title="Internet Relay Chat">IRC</abbr> to detect and prevent abuse and to
identify users connecting through shared networks.

## Features

* oidentd is highly [configurable][configuration], but configuration is
  optional and sensible defaults are provided.
* oidentd provides system administrators with a granular, capability-based
  [access control][acl] system.
* [Conditional replies][conditionals] enable users to send replies based on
  connection information, such as ports and IP addresses.
* oidentd is capable of sending [hidden][cap-hide], [randomized][cap-random],
  and [spoofed][cap-spoof] replies.
* oidentd can optionally handle requests for
  [<abbr title="Network Address Translation">NAT</abbr> connections][nat]
  and is capable of forwarding requests to other Ident servers.
* Both <abbr title="Internet Protocol version 4">IPv4</abbr> and
  <abbr title="Internet Protocol version 6">IPv6</abbr> are supported.
* oidentd is free software licensed under the
  <abbr title="GNU's Not Unix">GNU</abbr>
  <abbr title="General Public License, version 2">GPLv2</abbr>.

[acl]:           getting-started/configuration/index.md#capability-directives
[cap-hide]:      getting-started/capabilities.md#hide
[cap-random]:    getting-started/capabilities.md#random
[cap-spoof]:     getting-started/capabilities.md#spoof
[conditionals]:  getting-started/configuration/index.md#range-specification
[configuration]: getting-started/configuration/index.md
[nat]:           nat/introduction.md
