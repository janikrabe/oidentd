---
# Copyright (c)  2018-2020  Janik Rabe
#
# Permission is granted to copy, distribute and/or modify this document
# under the terms of the GNU Free Documentation License, Version 1.3
# or any later version published by the Free Software Foundation;
# with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
# A copy of the license is included in the file 'COPYING.DOC'

title: "Installation"
weight: 1
---

{{% alert "primary" "Looking for the download link?" %}}
You can download oidentd from the
[project website]({{% ref "/projects/oidentd" %}}).
{{% /alert %}}

Most popular operating system distributions include a recent version of oidentd
in their package repositories.
Installing oidentd using a package manager is recommended in most cases.

In some cases, however, it may be desirable to install oidentd from source.
This may be useful if your distribution does not package a recent version of
oidentd, or if there are any compile-time features you would like to enable.

More detailed instructions for compiling and installing oidentd can be found in
the `INSTALL` file included in all releases.

### Configuring the Build

After downloading, verifying and extracting oidentd, enter the directory you
extracted and run `./configure` to configure the build.

On many modern Linux systems, you may have to install libnetfilter\_conntrack
before running `./configure`.
More information can be found in the `INSTALL` file in the source tree.

The `./configure` script supports a number of optional flags:

* `--disable-ipv6` disables support for
  {{< abbr "IPv6" "Internet Protocol, Version 6" >}}.
* `--disable-libnfct` disables support for libnetfilter\_conntrack.
* `--disable-nat` disables support for
  {{< abbr "NAT" "Network Address Translation" >}}.
* `--enable-debug` compiles oidentd with the `--debug` option.
* `--enable-warn` is intended for developers and enables additional warning
  messages during compilation.

### Compile oidentd

Run `make` to compile oidentd.

You can run `src/oidentd --version` to verify that the compilation succeeded.

### Install oidentd

Run `make install` as root to install oidentd.

To uninstall oidentd later on, run `make uninstall` as root in the same
directory.
