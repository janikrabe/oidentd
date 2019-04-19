# Kernel Support Table

| Kernel            | IPv4 | IPv6 | IPv4 NAT | IPv6 NAT | Needs kmem | Needs root |
| ----------------- | ---- | ---- | -------- | -------- | ---------- | ---------- |
| DragonFly BSD     | Yes  | Yes  | ipf only | No       | NAT only   | If no NAT  |
| FreeBSD 5+        | Yes  | Yes  | ipf only | No       | NAT only   | If no NAT  |
| Linux             | Yes  | Yes  | Yes      | Yes      | No         | No         |
| NetBSD 5+         | Yes  | Yes  | No       | No       | No         | Yes        |
| OpenBSD 3+        | Yes  | Yes  | Yes      | No       | No         | No         |
