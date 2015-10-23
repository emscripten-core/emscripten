#include <signal.h>
#include <errno.h>
#include <string.h>
#include "syscall.h"
#include "pthread_impl.h"
#include "libc.h"
#include "ksigaction.h"

void __restore(), __restore_rt();

static pthread_t dummy(void) { return 0; }
weak_alias(dummy, __pthread_self_def);

static unsigned long handler_set[_NSIG/(8*sizeof(long))];

void __get_handler_set(sigset_t *set)
{
	memcpy(set, handler_set, sizeof handler_set);
}

int __libc_sigaction(int sig, const struct sigaction *restrict sa, struct sigaction *restrict old)
{
	struct k_sigaction ksa, ksa_old;
	if (sig >= (unsigned)_NSIG) {
		errno = EINVAL;
		return -1;
	}
	if (sa) {
		if ((uintptr_t)sa->sa_handler > 1UL) {
			a_or_l(handler_set+(sig-1)/(8*sizeof(long)),
				1UL<<(sig-1)%(8*sizeof(long)));
			__pthread_self_def();
		}
		ksa.handler = sa->sa_handler;
		ksa.flags = sa->sa_flags | SA_RESTORER;
		ksa.restorer = (sa->sa_flags & SA_SIGINFO) ? __restore_rt : __restore;
		memcpy(&ksa.mask, &sa->sa_mask, sizeof ksa.mask);
	}
	if (syscall(SYS_rt_sigaction, sig, sa?&ksa:0, old?&ksa_old:0, sizeof ksa.mask))
		return -1;
	if (old) {
		old->sa_handler = ksa_old.handler;
		old->sa_flags = ksa_old.flags;
		memcpy(&old->sa_mask, &ksa_old.mask, sizeof ksa_old.mask);
	}
	return 0;
}

int __sigaction(int sig, const struct sigaction *restrict sa, struct sigaction *restrict old)
{
	if (sig-32U < 3) {
		errno = EINVAL;
		return -1;
	}
	return __libc_sigaction(sig, sa, old);
}

weak_alias(__sigaction, sigaction);
