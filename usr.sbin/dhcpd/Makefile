# $OpenBSD: Makefile,v 1.7 2017/04/12 19:19:11 krw Exp $

.include <bsd.own.mk>

PREFIX?= /usr/local
BINDIR=  ${PREFIX}/sbin
LIBDIR=  ${PREFIX}/lib
MANDIR=  ${PREFIX}/share/man/man

SRCS=	bootp.c confpars.c db.c dhcp.c dhcpd.c bpf.c packet.c log.c \
	dispatch.c print.c memory.c options.c inet.c conflex.c parse.c \
	alloc.c tables.c tree.c hash.c convert.c icmp.c pfutils.c sync.c \
	udpsock.c
OS!=    uname -s
.if ${OS} == "FreeBSD"
SRCS+=os_compat_freebsd.c pfutils_compat_freebsd.c
.endif
PROG=	dhcpd
MAN=	dhcpd.8 dhcpd.conf.5 dhcpd.leases.5 dhcp-options.5

LDADD+=-lcrypto
.if ${OS} == "FreeBSD"
LDADD+=-lmd
OSREL!=uname -r | cut -d . -f 1
.if ${OSREL} >= 15
# FreeBSD 15+ needs nvlist library for pfutils code.
LDADD+=-lnv
.endif
.endif
CFLAGS+=-Wall

.include <bsd.prog.mk>
CFLAGS+=-Wstrict-prototypes -Wmissing-prototypes
CFLAGS+=-Wmissing-declarations
CFLAGS+=-Wshadow -Wpointer-arith -Wcast-qual
