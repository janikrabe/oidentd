<!--
Copyright (c)  2018-2020  Janik Rabe

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the file 'COPYING.DOC'
-->

# Starting the Server

oidentd can be run as a standalone server or started by a service manager.
By default, oidentd forks to the background after starting up.

If you installed oidentd from your distribution's package repositories, an
initialization script or [systemd service][systemd] may already have been
included in the package.
Consult your distribution's documentation for more information.

oidentd needs to be started as root on most systems, but normally drops its
privileges automatically after starting up.
See [Dropping Privileges][drop-privs] for details.

Run `oidentd` to start the server.

oidentd accepts a large number of command-line options.
Some commonly used options are:

* `-a`, `--address`: bind to the specified address (may be specified multiple
  times)
* `-d`, `--debug`: show messages that may be useful for debugging
* `-i`, `--foreground`: do not run as a background process
* `-p`, `--port`: listen on the specified port instead of port 113

The `oidentd(8)` manual page contains a complete list of options with detailed
explanations.
`oidentd --help` prints a list with more concise descriptions.

## systemd Service

Many systemd-based distributions include service files for oidentd.
On these distributions, oidentd can be started and enabled with the following
command:

```
systemctl enable --now oidentd
```

[drop-privs]: ../security/dropping-privileges.md
[systemd]:    #systemd-service
