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
