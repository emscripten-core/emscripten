#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "libc.h"
#include "atomic.h"

#define MAXTRIES 100

char *tempnam(const char *dir, const char *pfx)
{
	static int index;
	char *s;
	struct timespec ts;
	int pid = getpid();
	size_t l;
	int n;
	int try=0;

	if (!dir) dir = P_tmpdir;
	if (!pfx) pfx = "temp";

	if (access(dir, R_OK|W_OK|X_OK) != 0)
		return NULL;

	l = strlen(dir) + 1 + strlen(pfx) + 3*(sizeof(int)*3+2) + 1;
	s = malloc(l);
	if (!s) return s;

	do {
		clock_gettime(CLOCK_REALTIME, &ts);
		n = ts.tv_nsec ^ (uintptr_t)&s ^ (uintptr_t)s;
		snprintf(s, l, "%s/%s-%d-%d-%x", dir, pfx, pid, a_fetch_add(&index, 1), n);
	} while (!access(s, F_OK) && try++<MAXTRIES);
	if (try>=MAXTRIES) {
		free(s);
		return 0;
	}
	return s;
}
