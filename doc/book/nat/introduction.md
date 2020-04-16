---
# Copyright (c)  2018-2020  Janik Rabe
#
# Permission is granted to copy, distribute and/or modify this document
# under the terms of the GNU Free Documentation License, Version 1.3
# or any later version published by the Free Software Foundation;
# with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
# A copy of the license is included in the file 'COPYING.DOC'

title: "Introduction"
weight: 1
---

oidentd can handle Ident queries for other machines connecting through the
server it is running on.
This is especially useful when the server running oidentd performs Network
Address Translation (NAT).

Before configuring oidentd to use {{< abbr "NAT" "Network Address Translation"
>}}, please take a look at the `KERNEL_SUPPORT.md` file in the source tree to
find out whether {{< abbr "NAT" "Network Address Translation" >}} is supported
on your system.

oidentd supports two features that may be useful in combination with {{< abbr
"NAT" "Network Address Translation" >}}:

- [Static replies][static] instruct oidentd to send a certain reply in response
  to every Ident query intended for a particular host or subnetwork.
- [Forwarding][forwarding] allows Ident queries to be forwarded through {{<
  abbr "NAT" "Network Address Translation" >}} to the host that established a
  connection.

[static]:     {{% ref "static-replies" %}}
[forwarding]: {{% ref "forwarding" %}}
