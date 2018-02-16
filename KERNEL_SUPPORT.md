# Kernel Support Table

| Kernel            | IPv4 | IPv6 | IPv4 NAT | IPv6 NAT | Needs kmem |
| ----------------- | ---- | ---- | -------- | -------- | ---------- |
| Darwin            | Yes  | Yes  | ipf only | No       | Yes        |
| DragonFly BSD     | Yes  | Yes  | ipf only | No       | NAT only   |
| FreeBSD 1-3       | Yes  | Yes  | ipf only | No       | Yes        |
| FreeBSD 4         | Yes  | Yes  | ipf only | No       | NAT only   |
| FreeBSD 5+        | Yes  | Yes  | ipf only | No       | NAT only   |
| Linux             | Yes  | Yes  | Yes      | No       | No         |
| NetBSD 1-4        | Yes  | Yes  | ipf only | No       | Yes        |
| NetBSD 5+         | Yes  | Yes  | No       | No       | No         |
| OpenBSD 2.0 - 2.3 | Yes  | Yes  | ipf only | No       | Yes        |
| OpenBSD 2.4 - 2.8 | Yes  | Yes  | ipf only | No       | NAT only   |
| OpenBSD 2.9       | Yes  | Yes  | ipf only | No       | NAT only   |
| OpenBSD 3+        | Yes  | Yes  | Yes      | No       | No         |
| Solaris 2.4       | Yes  | No   | No       | No       | Yes        |
| Solaris 2.5       | Yes  | No   | No       | No       | Yes        |
| Solaris 2.6 - 2.7 | Yes  | No   | No       | No       | Yes        |
| Solaris 2.8       | Yes  | No   | No       | No       | Yes        |
