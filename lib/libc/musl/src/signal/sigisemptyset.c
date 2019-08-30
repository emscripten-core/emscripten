#define _GNU_SOURCE
#include <signal.h>
#include <string.h>

int sigisemptyset(const sigset_t *set)
{
	static const unsigned long zeroset[_NSIG/8/sizeof(long)];
	return !memcmp(set, &zeroset, _NSIG/8);
}
