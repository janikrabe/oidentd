# Kernel Support Table

| Kernel            | IPv4 | IPv6 | IPv4 NAT | IPv6 NAT | Needs kmem | Needs root |
| ----------------- | ---- | ---- | -------- | -------- | ---------- | ---------- |
| Darwin            | Yes  | Yes  | ipf only | No       | Yes        | No         |
| DragonFly BSD     | Yes  | Yes  | ipf only | No       | NAT only   | If no NAT  |
| FreeBSD 1-3       | Yes  | Yes  | ipf only | No       | Yes        | No         |
| FreeBSD 4         | Yes  | Yes  | ipf only | No       | NAT only   | If no NAT  |
| FreeBSD 5+        | Yes  | Yes  | ipf only | No       | NAT only   | If no NAT  |
| Linux             | Yes  | Yes  | Yes      | No       | No         | No         |
| NetBSD 1-4        | Yes  | Yes  | ipf only | No       | Yes        | No         |
| NetBSD 5+         | Yes  | Yes  | No       | No       | No         | Yes        |
| OpenBSD 2.0 - 2.3 | Yes  | Yes  | ipf only | No       | Yes        | No         |
| OpenBSD 2.4 - 2.8 | Yes  | Yes  | ipf only | No       | NAT only   | No         |
| OpenBSD 2.9       | Yes  | Yes  | ipf only | No       | NAT only   | No         |
| OpenBSD 3+        | Yes  | Yes  | Yes      | No       | No         | No         |
| Solaris 2.4       | Yes  | No   | No       | No       | Yes        | No         |
| Solaris 2.5       | Yes  | No   | No       | No       | Yes        | No         |
| Solaris 2.6 - 2.7 | Yes  | No   | No       | No       | Yes        | No         |
| Solaris 2.8       | Yes  | No   | No       | No       | Yes        | No         |
