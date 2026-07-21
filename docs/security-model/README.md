# Security model

DHCP is a tricky protocol to work with because it requires the speaker to
generate both layer 2 (link layer) Ethernet frames and layer 3 (network
layer) IP packets. Working with Ethernet frames is required because clients
start with no IP address and thus cannot create valid IP packets until
they are assigned an address by a DHCP server. Working in L2 necessitates
the use of raw sockets which means the speaker process must run as root.

## Upstream security mechanisms

On OpenBSD, dhcpd uses a combination of mechanisms to lower the potential
blast radius of security bugs:

- Opens file descriptors and sockets as root and then switches to
  a non-root user before processing input data from them
- Restricts the raw socket's functionality from being modified using
  the BPF BIOCLOCK IOCTL
- Sets the process' file system root to an empty directory using `chroot(2)`
- Calls [pledge(2)][pledge-manual] on startup to limit the system calls
  that the process can make
- Calls [unveil(2)][unveil-manual] when pf support is enabled for the
  pfutils helper process to disable file system access

FreeBSD has no equivalent to `pledge` or `unveil`, but most of the other
mechanisms can be reused without modification.

[pledge-manual]: https://man.openbsd.org/pledge
[unveil-manual]: https://man.openbsd.org/unveil

## freedhcpd security mechanisms

On the topic of parsing data as root, the OpenBSD code parses an address
lease database as root. The lease database contains some attacker-controlled
data such as machine hostnames. I have updated that code to parse the list
after switching to a non-root user ([seen here][lease-db-commit]). I tried
to [upstream my change][lease-db-upstream], but I did not receive any
feedback on it.

FreeBSD does not have direct equivalents to `pledge` and `unveil`. Instead,
FreeBSD offers two sandboxing mechanisms: jails and capsicum.

Jails can be thought of as "containers". They are usually instantiated by
creating a copy of the FreeBSD userland in a directory and defining a jail
configuration file that starts the init system from that userland. All child
processes of the initial jail process are automatically "jailed". As a
result, regular users and sysadmins can sandbox programs without modifying
their source code. Programs can also invoke the jail APIs directly via
system calls and libc. For our purposes though, jails are a big hammer
compared to capsicum.

capsicum cannot be applied to a program without modifying the program's
source code. However, capsicum provides much more granular control over
what the program has access to and does not require a FreeBSD userland
or a minimalistic copy of the userland to run. capsicum attempts to
implement [capability-based security][capability-based-security] by
extending file descriptors into capabilities. The general pattern of
a capsicum-enabled program is that it acquires access to all its
required capabilities on startup and calls `cap_enter(2)`. From that
point forward, the process cannot acquire new capabilities. In addition,
restrictions are automatically placed on the already-acquired
capabilities to prevent the application from escaping its sandbox.

The term "capability" usually just means a file descriptor or a socket.
In freedhcpd's case, its raw socket / BPF device would be considered
a single capability. FreeBSD additionally slices the operating system
into namespaces. By default, capsicum prevents processes from accessing
the "global namespaces" *after* cap_enter is called. Unfortunately,
what is considered a logical namespace is a bit nebulous, so simply
opening a bunch of file descriptors and calling cap_enter may prove
to be too restrictive by default.

Granular control of what a capability can and cannot do is controlled
using [capability rights][rights-manual]. Capability rights bare some
similarity to pledge(2) by restricting system calls and functionality,
but for a single resource. 

freedhcpd makes extensive use of capability rights to restrict what it
can do with the files and the various sockets it opens. Implementing
these restrictions was mostly a matter of manually testing dhcpd,
looking for error messages, and reviewing the [rights(4)][rights-manual]
manual. freedhcpd's capabilities can be viewed at runtime by executing:

```sh
procstat -fC $(pgrep dhcpd)
```

[lease-db-commit]: https://codeberg.org/stephen-fox/freedhcpd/commit/1f08fc5be2c21a78fb940974fb577fa6b716b361
[lease-db-upstream]: https://marc.info/?l=openbsd-tech&m=170277830117415&w=2
[capability-based-security]: https://en.wikipedia.org/wiki/Capability-based_security
[rights-manual]: https://man.freebsd.org/cgi/man.cgi?query=rights
