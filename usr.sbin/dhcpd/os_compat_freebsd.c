#include <net/if.h>     // Needed by: get_iface_fib
#include <netinet/in.h> // Needed for: INADDR_BROADCAST, struct sockaddr_in
#include <sys/types.h>  // Needed by: getrtable
#include <sys/sysctl.h> // Needed by: getrtable
#include <sys/user.h>   // Needed by: getrtable
#include <sys/ioctl.h>  // Needed by: get_iface_fib
#include <sys/socket.h> // Needed by: get_iface_fib
#include <stdio.h>      // Needed by: getrtable
#include <stdlib.h>     // Needed by: getrtable
#include <string.h>     // Needed by: get_iface_fib
#include <unistd.h>     // Needed by: getrtable

#include "log.h" // Needed by custom functions.

#include "os_compat.h"

// pledge is an OpenBSD thing, this is just a placeholder.
int
pledge(const char *promises, const char *execpromises)
{
	(void)promises;
	(void)execpromises;
	return 0;
}

// unveil is an OpenBSD thing, this is just a placeholder.
int
unveil(const char *path, const char *permissions)
{
	(void)path;
	(void)permissions;
	return 0;
}

// getrtable stands in for OpenBSD's getrtable(2). This version uses
// sysctl(3) to get the current process' FIB (the equivalent to rtable).
int
getrtable(void)
{
	struct kinfo_proc kp;
	size_t len = sizeof(kp);
	int mib[4];

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = getpid();

	if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1)
		fatal("get process fib");

	return kp.ki_fibnum;
}

// get_iface_fib returns the FIB of a network interface. This code is
// essentially a copy of the "get_rdomain" function that was removed
// in OpenBSD commit 9d7a36478719f85f4d082aabc1772c51f9f26d8d.
int
get_iface_fib(char *name)
{
	int rv = 0, s;
	struct  ifreq ifr;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		fatal("get_iface_fib socket");

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFFIB, (caddr_t)&ifr) != -1)
		rv = ifr.ifr_fib;

	close(s);
	return rv;
}
