#include <unistd.h>
#include <string.h>
#include "syscall.h"
#include "libc.h"
#include "pthread_impl.h"

static void dummy(int x)
{
}

weak_alias(dummy, __fork_handler);

pid_t fork(void)
{
	pid_t ret;
	sigset_t set;
	__fork_handler(-1);
	__block_all_sigs(&set);
	ret = syscall(SYS_fork);
	if (libc.main_thread && !ret) {
		pthread_t self = __pthread_self();
		self->tid = self->pid = syscall(SYS_getpid);
		memset(&self->robust_list, 0, sizeof self->robust_list);
		libc.threads_minus_1 = 0;
		libc.main_thread = self;
	}
	__restore_sigs(&set);
	__fork_handler(!ret);
	return ret;
}
