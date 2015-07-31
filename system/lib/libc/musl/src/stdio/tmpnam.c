#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "libc.h"
#include "syscall.h"
#include "atomic.h"

#define MAXTRIES 100

char *tmpnam(char *s)
{
	static int index;
	static char s2[L_tmpnam];
	struct timespec ts;
	int try = 0;
	unsigned n;

	if (!s) s = s2;

	if (__syscall(SYS_access, P_tmpdir, R_OK|W_OK|X_OK) != 0)
		return NULL;

	do {
		__syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts, 0);
		n = ts.tv_nsec ^ (uintptr_t)&s ^ (uintptr_t)s;
		snprintf(s, L_tmpnam, "/tmp/t%x-%x", a_fetch_add(&index, 1), n);
	} while (!__syscall(SYS_access, s, F_OK) && try++<MAXTRIES);
	return try>=MAXTRIES ? 0 : s;
}
