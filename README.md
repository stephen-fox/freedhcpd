# freedhcpd

freedhcpd is a port of OpenBSD's IPv4 DHCP server ([dhcpd][openbsd-dhcpd])
for FreeBSD with [capsicum(4)][capsicum-manual] support and some other small
changes. The goal of freedhcpd is to provide a simple, security-conscious
DHCP server for FreeBSD with the smallest number of changes required to make
it work properly on FreeBSD. This repository also retains the revision
history of the ISC dhcpd code, on which OpenBSD's dhcpd is based.

[openbsd-dhcpd]: https://codeberg.org/OpenBSD/src/src/branch/master/usr.sbin/dhcpd
[capsicum-manual]: https://man.freebsd.org/cgi/man.cgi?query=capsicum

## Documentation

- [Security model](docs/security-model/README.md) - Overview of OpenBSD's
  dhcpd security model and freedhcpd's changes
- [Development documentation](docs/development/README.md) - Developer
  resources, including building and testing instructions
- [History](docs/history/README.md) - Notes about preserving the revision
  history of ISC dhcpd and OpenBSD dhcpd
