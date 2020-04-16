---
# Copyright (c)  2018-2020  Janik Rabe
#
# Permission is granted to copy, distribute and/or modify this document
# under the terms of the GNU Free Documentation License, Version 1.3
# or any later version published by the Free Software Foundation;
# with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
# A copy of the license is included in the file 'COPYING.DOC'

title: "Forwarding"
weight: 3
---

oidentd can forward Ident queries to the host they were intended for, provided
that this host is connecting through the machine oidentd is running on.

When forwarding is enabled, the default behavior is to forward immediately and
only fall back to using the configured [static replies][static-replies] if
forwarding fails.
This can be changed by using the `--masquerade-first` (`-M`) flag, in which
case oidentd only forwards requests if no matching static reply was found.

## Configuring the Gateway

Forwarding can be enabled on the device performing network address translation
by running oidentd with the `--forward` (`-f`) option.
Optionally, a target port may be specified using `--forward=port`.
If no port is specified, port `113` is used.

## Configuring the Servers

All machines that need to receive forwarded queries must be running an Ident
server capable of handling these queries, such as oidentd with the `--proxy`
(`-P`) option.
For example, oidentd can be run on a machine behind {{< abbr "NAT"
"Network Address Translation" >}} with the following command:

{{< code >}}
oidentd -P 10.0.0.1
{{< /code >}}

This allows `10.0.0.1` to forward queries to the current oidentd instance.

If you specified a custom target port for forwarding, make sure the target
server is configured to listen on that port:

{{< code >}}
oidentd -P 10.0.0.1 -p 113
{{< /code >}}

Port `113` is the default and does not need to be specified explicitly.

[static-replies]: {{% ref "static-replies" %}}
