// __dead is an OpenBSD C compiler macro. The advantage of defining
// a placeholder here is IDEs and other high-level tools can learn
// of its existence (versus defining it in the Makefile, which said
// tools may not parse).
#ifndef __dead
#define __dead
#endif

// Required for the following symbols in FreeBSD:
// - INADDR_BROADCAST
// - struct sockaddr_in
#include <netinet/in.h>

// setfib is the equivalent to OpenBSD setrtable.
#define setrtable(rdomain) setfib(rdomain)

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
