/*
 * Stub functions for testing pfutils_compat_freebsd.c
 * Provides minimal implementations of dhcpd functions
 */

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Stub for log_warn - just print to stderr */
void
log_warn(const char *fmt, ...)
{
	va_list ap;
	char *msg;

	va_start(ap, fmt);
	if (vasprintf(&msg, fmt, ap) == -1) {
		vwarn(fmt, ap);
	} else {
		if (errno != 0)
			warn("%s", msg);
		else
			warnx("%s", msg);
		free(msg);
	}
	va_end(ap);
}
