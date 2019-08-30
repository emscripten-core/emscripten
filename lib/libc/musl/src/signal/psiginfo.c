#include <stdio.h>
#include <string.h>
#include <signal.h>

void psiginfo(const siginfo_t *si, const char *msg)
{
	char *s = strsignal(si->si_signo);
	if (msg) fprintf(stderr, "%s: %s\n", msg, s);
	else fprintf(stderr, "%s\n", s);
}
