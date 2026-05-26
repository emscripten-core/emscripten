#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "syscall.h"

int sigaltstack(const stack_t *restrict ss, stack_t *restrict old)
{
	if (ss) {
		size_t min = sysconf(_SC_MINSIGSTKSZ);
		if (!(ss->ss_flags & SS_DISABLE) && ss->ss_size < min) {
			errno = ENOMEM;
			return -1;
		}
		if (ss->ss_flags & SS_ONSTACK) {
			errno = EINVAL;
			return -1;
		}
	}
	return syscall(SYS_sigaltstack, ss, old);
}
