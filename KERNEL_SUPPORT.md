# Kernel Support Table

| Kernel                         | IPv4 | IPv6 | IPv4 NAT | IPv6 NAT | Needs kmem | Notes                    |
| ------------------------------ | ---- | ---- | -------- | -------- | ---------- | ------------------------ |
| [Darwin][darwin]               | Yes  | Yes  | ipf only | No       | Yes        |                          |
| [DragonFly BSD][freebsd4]      | Yes  | Yes  | ipf only | No       | NAT only   |                          |
| [FreeBSD 1-3][freebsd]         | Yes  | Yes  | ipf only | No       | Yes        |                          |
| [FreeBSD 4][freebsd4]          | Yes  | Yes  | ipf only | No       | NAT only   | Ignores `--forward-last` |
| [FreeBSD 5+][freebsd5]         | Yes  | Yes  | ipf only | No       | NAT only   |                          |
| [Linux][linux]                 | Yes  | Yes  | Yes      | No       | No         |                          |
| [NetBSD 1-4][netbsd]           | Yes  | Yes  | ipf only | No       | Yes        |                          |
| [NetBSD 5+][netbsd5]           | Yes  | Yes  | No       | No       | No         |                          |
| [OpenBSD 2.0 - 2.3][openbsd]   | Yes  | Yes  | ipf only | No       | Yes        |                          |
| [OpenBSD 2.4 - 2.8][openbsd24] | Yes  | Yes  | ipf only | No       | NAT only   | Ignores `--forward-last` |
| [OpenBSD 2.9][openbsd29]       | Yes  | Yes  | ipf only | No       | NAT only   | Ignores `--forward-last` |
| [OpenBSD 3+][openbsd30]        | Yes  | Yes  | Yes      | No       | No         |                          |
| [Solaris 2.4][solaris4]        | Yes  | No   | No       | No       | Yes        |                          |
| [Solaris 2.5][solaris5]        | Yes  | No   | No       | No       | Yes        |                          |
| [Solaris 2.6 - 2.7][solaris7]  | Yes  | No   | No       | No       | Yes        |                          |
| [Solaris 2.8][solaris8]        | Yes  | No   | No       | No       | Yes        |                          |

Links do not necessarily indicate which source file is being used.
See `configure.ac` for which file is compiled on any given system.

[darwin]:    src/kernel/darwin.c
[freebsd]:   src/kernel/freebsd.c
[freebsd4]:  src/kernel/freebsd4.c
[freebsd5]:  src/kernel/freebsd5.c
[linux]:     src/kernel/linux.c
[netbsd]:    src/kernel/netbsd.c
[netbsd5]:   src/kernel/netbsd5.c
[openbsd]:   src/kernel/openbsd.c
[openbsd24]: src/kernel/openbsd24.c
[openbsd29]: src/kernel/openbsd29.c
[openbsd30]: src/kernel/openbsd30.c
[solaris4]:  src/kernel/solaris4.c
[solaris5]:  src/kernel/solaris5.c
[solaris7]:  src/kernel/solaris7.c
[solaris8]:  src/kernel/solaris8.c
