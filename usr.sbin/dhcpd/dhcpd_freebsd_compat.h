// __dead is an OpenBSD C compiler macro. The advantage of defining
// a placeholder here is IDEs and other high-level tools can learn
// of its existence (versus defining it in the Makefile, which said
// tools may not parse).
#ifndef __dead
#define __dead
#endif

#include <sys/types.h>  // Needed by: getrtable
#include <sys/sysctl.h> // Needed by: getrtable
#include <sys/user.h>   // Needed by: getrtable
#include <stdio.h>      // Needed by: getrtable
#include <stdlib.h>     // Needed by: getrtable
#include <unistd.h>     // Needed by: getrtable
#include <err.h>        // Needed by: getrtable

// Required for the following symbols in FreeBSD:
// - INADDR_BROADCAST
// - struct sockaddr_in
#include <netinet/in.h>

// pledge is an OpenBSD thing, this is just a placeholder.
#ifdef pledge
#error "pledge already defined"
#else
static inline int pledge(const char *promises, const char *execpromises) {
	(void)promises;
	(void)execpromises;
	return 0;
}
#endif

// getrtable stands in for OpenBSD's getrtable(2). This version uses
// sysctl(3) to get the current process' FIB (the equivalent to rtable).
static inline int
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
                err(1, "get process fib");

        return kp.ki_fibnum;
}
