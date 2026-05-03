/*
 * Test program for pf_kill_state function
 * Links with pfutils_compat_freebsd.c
 *
 * Usage: test_pf_kill <IP_ADDRESS>
 * Example: test_pf_kill 10.0.0.100
 */

#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Declaration - implementation in pfutils_compat_freebsd.c */
void pf_kill_state(int fd, struct in_addr ip);

static void
usage(void)
{
	fprintf(stderr, "Usage: test_pf_kill <IP_ADDRESS>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Kill all pf states associated with an IP address.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "  test_pf_kill 10.0.0.100\n");
	fprintf(stderr, "  test_pf_kill 192.168.1.50\n");
	fprintf(stderr, "\n");
#if __FreeBSD_version < 1500000
	fprintf(stderr, "Implementation: DIOCKILLSTATES (FreeBSD 14)\n");
#else
	fprintf(stderr, "Implementation: DIOCKILLSTATESNV (FreeBSD 15+)\n");
#endif
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct in_addr ip;
	int fd;

	if (argc != 2)
		usage();

	/* Parse IP address */
	if (inet_aton(argv[1], &ip) == 0)
		errx(1, "Invalid IP address: %s", argv[1]);

	printf("Testing pf_kill_state() for IP: %s\n", inet_ntoa(ip));
#if __FreeBSD_version < 1500000
	printf("FreeBSD version: %d (DIOCKILLSTATES path)\n", __FreeBSD_version);
#else
	printf("FreeBSD version: %d (DIOCKILLSTATESNV path)\n", __FreeBSD_version);
#endif
	printf("\n");

	/* Open pf device */
	fd = open("/dev/pf", O_RDWR);
	if (fd == -1)
		err(1, "Cannot open %s (are you root?)", "/dev/pf");

	/* Show current states */
	printf("Before: Check current states with:\n");
	printf("  pfctl -ss | grep %s\n\n", inet_ntoa(ip));

	/* Call pf_kill_state from pfutils_compat_freebsd.c */
	printf("Calling pf_kill_state()...\n");
	pf_kill_state(fd, ip);

	close(fd);

	printf("\nAfter: Verify states were killed with:\n");
	printf("  pfctl -ss | grep %s\n", inet_ntoa(ip));

	return (0);
}
