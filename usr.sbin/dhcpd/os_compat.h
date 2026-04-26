// __dead is an OpenBSD C compiler macro. The advantage of defining
// a placeholder here is IDEs and other high-level tools can learn
// of its existence (versus defining it in the Makefile, which said
// tools may not parse).
#ifndef __dead
#define __dead
#endif

#ifndef M_IPV4_CSUM_IN_OK
// M_IPV4_CSUM_IN_OK is an OpenBSD-specific constant which
// does not have a FreeBSD equivalent.
// Value taken from OpenBSD: /usr/include/sys/mbuf.h:
#define	M_IPV4_CSUM_IN_OK	0x0008	/* IPv4 checksum verified */
#endif

#ifndef M_UDP_CSUM_IN_OK
// M_UDP_CSUM_IN_OK is an OpenBSD-specific constant which
// does not have a FreeBSD equivalent.
// Value taken from OpenBSD: /usr/include/sys/mbuf.h:
#define	M_UDP_CSUM_IN_OK	0x0080	/* UDP checksum verified */
#endif

int pledge(const char *promises, const char *execpromises);

int unveil(const char *path, const char *permissions);

int getrtable(void);

int get_iface_fib(char *name);
