# freedhcpd

freedhcpd is a port of OpenBSD's IPv4 DHCP server ([dhcpd][openbsd-dhcpd])
for FreeBSD with [capsicum(4)][capsicum-manual] sandbox support and some
other small changes.

The goal of freedhcpd is to provide a simple, security-conscious DHCP server
for FreeBSD with the smallest number of changes required to make it work
properly on FreeBSD.

OpenBSD's dhcpd is a fork of the ISC dhcpd that was started in 2004. This
repository retains the revision history of both the OpenBSD code and the
original ISC dhcpd code going back to 1995.

[openbsd-dhcpd]: https://codeberg.org/OpenBSD/src/src/branch/master/usr.sbin/dhcpd
[capsicum-manual]: https://man.freebsd.org/cgi/man.cgi?query=capsicum

## Project status

While I have been working on freedhcpd for a while now and running it for
over two years on my home network, I am not doing anything super complicated
with it.

There are some minor issues like ICMP messages not being sent due to
capsicum restrictions. I am not thoroughly testing all of dhcpd's
features. In its current state, I would say only use this code if
you want to experiment.

## Documentation

- [Security model](docs/security-model/README.md) - Overview of OpenBSD's
  dhcpd security model and freedhcpd's changes
- [Development documentation](docs/development/README.md) - Developer
  resources, including building and testing instructions
- [History](docs/history/README.md) - Notes about preserving the revision
  history of ISC dhcpd and OpenBSD dhcpd
