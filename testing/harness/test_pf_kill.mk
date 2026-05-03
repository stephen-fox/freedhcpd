# Makefile for testing pf_kill_state implementation

PROG=   test_pf_kill

# Source files
SRCS=   test_pf_kill.c \
        pfutils_compat_freebsd.c \
        test_stubs.c

# Get OS version
OS!=    uname -s

# Only build on FreeBSD
.if ${OS} != "FreeBSD"
.error This test only works on FreeBSD
.endif

# Compiler flags
CFLAGS+=        -Wall -Wextra -Werror
# Include current directory first for stub headers, then dhcpd directory
CFLAGS+=        -I. -I../../usr.sbin/dhcpd

# FreeBSD 15+ needs nvlist library
OSREL!= uname -r | cut -d. -f1
.if ${OSREL} >= 15
LDADD+= -lnv
.endif

# Path to pfutils_compat_freebsd.c
VPATH=  ../../usr.sbin/dhcpd

# No man page for this test program
MAN=

.include <bsd.prog.mk>
