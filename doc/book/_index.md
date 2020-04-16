---
# Copyright (c)  2018-2020  Janik Rabe
#
# Permission is granted to copy, distribute and/or modify this document
# under the terms of the GNU Free Documentation License, Version 1.3
# or any later version published by the Free Software Foundation;
# with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
# A copy of the license is included in the file 'COPYING.DOC'

title: "About oidentd"
---

{{% project-detail "description" %}}.

oidentd is a configurable, {{< abbr "RFC" "Request for Comments" >}} 1413
compliant Ident server.
It runs on Linux, FreeBSD, OpenBSD, NetBSD, and DragonFly BSD.

The Ident Protocol is used primarily on
{{< abbr "IRC" "Internet Relay Chat" >}} to detect and prevent abuse and to
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
  [{{< abbr "NAT" "Network Address Translation" >}} connections][nat]
  and is capable of forwarding requests to other Ident servers.
* Both {{< abbr "IPv4" "Internet Protocol, Version 4" >}} and
  {{< abbr "IPv6" "Internet Protocol, Version 6" >}} are supported.
* oidentd is free software licensed under the
  {{< abbr "GNU" "GNU's Not Unix" >}}
  {{< abbr "GPLv2" "General Public License, Version 2" >}}.

[acl]:           {{% ref "getting-started/configuration#capability-directives" %}}
[cap-hide]:      {{% ref "getting-started/capabilities#hide" %}}
[cap-random]:    {{% ref "getting-started/capabilities#random" %}}
[cap-spoof]:     {{% ref "getting-started/capabilities#spoof" %}}
[conditionals]:  {{% ref "getting-started/configuration#range-specification" %}}
[configuration]: {{% ref "getting-started/configuration" %}}
[nat]:           {{% ref "nat/introduction" %}}
