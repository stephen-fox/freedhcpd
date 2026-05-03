/*
 * FreeBSD-specific compatibility layer for pf_kill_state
 * Handles differences between FreeBSD 14 (DIOCKILLSTATES)
 * and FreeBSD 15+ (DIOCKILLSTATESNV with nvlist).
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <net/if.h>
#include <net/pfvar.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "log.h"

#include "os_compat.h"

#if __FreeBSD_version < 1500000
/*
 * FreeBSD 14 and earlier - use DIOCKILLSTATES ioctl
 * with struct pfioc_state_kill. This code is a copy
 * of the pf_kill_state function from pfutils.c at
 * OpenBSD commit:
 * 52d5f036b15e8cd226a9f9394d76bd61596f8d63
 */
void
pf_kill_state(int fd, struct in_addr ip)
{
	struct pfioc_state_kill	psk;
	struct pf_addr target;

	memset(&psk, 0, sizeof(psk));
	memset(&target, 0, sizeof(target));

	memcpy(&target.v4, &ip.s_addr, 4);
	psk.psk_af = AF_INET;

	/* Kill all states from target */
	memcpy(&psk.psk_src.addr.v.a.addr, &target,
	    sizeof(psk.psk_src.addr.v.a.addr));
	memset(&psk.psk_src.addr.v.a.mask, 0xff,
	    sizeof(psk.psk_src.addr.v.a.mask));
	if (ioctl(fd, DIOCKILLSTATES, &psk) == -1) {
		log_warn("DIOCKILLSTATES failed (src)");
	}

	/* Kill all states to target */
	memset(&psk.psk_src, 0, sizeof(psk.psk_src));
	memcpy(&psk.psk_dst.addr.v.a.addr, &target,
	    sizeof(psk.psk_dst.addr.v.a.addr));
	memset(&psk.psk_dst.addr.v.a.mask, 0xff,
	    sizeof(psk.psk_dst.addr.v.a.mask));
	if (ioctl(fd, DIOCKILLSTATES, &psk) == -1) {
		log_warn("DIOCKILLSTATES failed (dst)");
	}
}

#else
/*
 * FreeBSD 15 and above - use DIOCKILLSTATESNV ioctl with nvlist.
 * In FreeBSD 15, the DIOCKILLSTATES ioctl was removed. This code
 * mimics the FreeBSD kernel's pf_nvstate_kill_to_kstate_kill
 * function's expectations from sys/netpfil/pf/pf_nv.c at FreeBSD
 * commit: bbfdabc12895ce2538444747684c6a4fe53298ba
 */
#include <sys/nv.h>

/* Address type constants from pf.h */
#define PF_ADDR_ADDRMASK	0

static nvlist_t *
create_pf_addr_nvlist(struct in_addr *addr)
{
	nvlist_t *nvl;
	struct pf_addr pfa;

	nvl = nvlist_create(0);
	if (nvl == NULL)
		return (NULL);

	/* Pack IPv4 address into pf_addr structure */
	memset(&pfa, 0, sizeof(pfa));
	memcpy(&pfa.v4, addr, sizeof(*addr));

	nvlist_add_binary(nvl, "addr", &pfa, sizeof(pfa));

	return (nvl);
}

static nvlist_t *
create_addr_wrap_nvlist(struct in_addr ip, int has_addr)
{
	nvlist_t *nvl, *addr_nvl, *mask_nvl;
	struct in_addr mask;

	nvl = nvlist_create(0);
	if (nvl == NULL)
		return (NULL);

	/* Set address type to ADDRMASK */
	nvlist_add_number(nvl, "type", PF_ADDR_ADDRMASK);
	nvlist_add_number(nvl, "iflags", 0);

	/* Create addr sub-nvlist */
	if (has_addr) {
		addr_nvl = create_pf_addr_nvlist(&ip);
		/* Full /32 mask */
		mask.s_addr = 0xffffffff;
	} else {
		/* Empty address */
		struct in_addr any_addr = { 0 };
		addr_nvl = create_pf_addr_nvlist(&any_addr);
		/* Empty mask */
		mask.s_addr = 0;
	}

	if (addr_nvl == NULL) {
		nvlist_destroy(nvl);
		return (NULL);
	}

	nvlist_add_nvlist(nvl, "addr", addr_nvl);
	nvlist_destroy(addr_nvl);

	/* Create mask sub-nvlist */
	mask_nvl = create_pf_addr_nvlist(&mask);
	if (mask_nvl == NULL) {
		nvlist_destroy(nvl);
		return (NULL);
	}

	nvlist_add_nvlist(nvl, "mask", mask_nvl);
	nvlist_destroy(mask_nvl);

	return (nvl);
}

static nvlist_t *
create_rule_addr_nvlist(struct in_addr ip, int has_addr)
{
	nvlist_t *nvl, *addr_wrap_nvl;

	nvl = nvlist_create(0);
	if (nvl == NULL)
		return (NULL);

	/* Create the addr_wrap sub-nvlist */
	addr_wrap_nvl = create_addr_wrap_nvlist(ip, has_addr);
	if (addr_wrap_nvl == NULL) {
		nvlist_destroy(nvl);
		return (NULL);
	}

	nvlist_add_nvlist(nvl, "addr", addr_wrap_nvl);
	nvlist_destroy(addr_wrap_nvl);

	/* Add port array (both elements 0 for no port filtering) */
	nvlist_append_number_array(nvl, "port", 0);
	nvlist_append_number_array(nvl, "port", 0);

	/* Add other required fields */
	nvlist_add_number(nvl, "neg", 0);      /* not negated */
	nvlist_add_number(nvl, "port_op", 0);  /* no port operation */

	return (nvl);
}

static int
kill_states_nv(int fd, struct in_addr ip, int kill_src)
{
	struct pfioc_nv nv;
	nvlist_t *nvl, *cmp_nvl, *src_nvl, *dst_nvl;
	void *packed = NULL;
	int error = 0;
	struct in_addr any_addr = { 0 };

	/* Create main nvlist for state kill request */
	nvl = nvlist_create(0);
	if (nvl == NULL)
		return (-1);

	/* Create comparison mode nvlist */
	cmp_nvl = nvlist_create(0);
	if (cmp_nvl == NULL) {
		nvlist_destroy(nvl);
		return (-1);
	}

	nvlist_add_number(cmp_nvl, "id", 0);
	nvlist_add_number(cmp_nvl, "creatorid", 0);
	nvlist_add_number(cmp_nvl, "direction", 0);

	nvlist_add_nvlist(nvl, "cmp", cmp_nvl);
	nvlist_destroy(cmp_nvl);

	/* Set address family and protocol (0 = all protocols) */
	nvlist_add_number(nvl, "af", AF_INET);
	nvlist_add_number(nvl, "proto", 0);

	/* Create source and destination rule_addr nvlists */
	if (kill_src) {
		/* Kill states FROM this IP */
		src_nvl = create_rule_addr_nvlist(ip, 1);
		dst_nvl = create_rule_addr_nvlist(any_addr, 0);
	} else {
		/* Kill states TO this IP */
		src_nvl = create_rule_addr_nvlist(any_addr, 0);
		dst_nvl = create_rule_addr_nvlist(ip, 1);
	}

	if (src_nvl == NULL || dst_nvl == NULL) {
		nvlist_destroy(nvl);
		nvlist_destroy(src_nvl);
		nvlist_destroy(dst_nvl);
		return (-1);
	}

	nvlist_add_nvlist(nvl, "src", src_nvl);
	nvlist_add_nvlist(nvl, "dst", dst_nvl);
	nvlist_destroy(src_nvl);
	nvlist_destroy(dst_nvl);

	/* Add empty strings for ifname and label */
	nvlist_add_string(nvl, "ifname", "");
	nvlist_add_string(nvl, "label", "");

	/* Set kill_match to true to actually kill the states */
	nvlist_add_bool(nvl, "kill_match", true);

	/* Pack the nvlist for kernel consumption */
	packed = nvlist_pack(nvl, &nv.len);
	if (packed == NULL) {
		nvlist_destroy(nvl);
		return (-1);
	}

	nv.data = packed;
	nv.size = nv.len;

	/* Execute the ioctl */
	if (ioctl(fd, DIOCKILLSTATESNV, &nv) == -1) {
		error = -1;
	}

	free(packed);
	nvlist_destroy(nvl);
	return (error);
}

void
pf_kill_state(int fd, struct in_addr ip)
{
	/* Kill all states from target */
	if (kill_states_nv(fd, ip, 1) == -1)
		log_warn("DIOCKILLSTATESNV failed (src)");

	/* Kill all states to target */
	if (kill_states_nv(fd, ip, 0) == -1)
		log_warn("DIOCKILLSTATESNV failed (dst)");
}

#endif /* __FreeBSD_version < 1500000 */
