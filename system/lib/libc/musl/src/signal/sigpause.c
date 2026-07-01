#include <signal.h>

int sigpause(int sig)
{
	sigset_t mask;
	sigprocmask(0, 0, &mask);
	if (sigdelset(&mask, sig)) return -1;
	return sigsuspend(&mask);
}
