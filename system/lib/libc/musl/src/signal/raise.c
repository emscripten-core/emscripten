#include <signal.h>
#include <stdint.h>
#include "syscall.h"
#include "pthread_impl.h"

int raise(int sig)
{
	int pid, tid, ret;
	sigset_t set;
	__block_app_sigs(&set);
	tid = __syscall(SYS_gettid);
	pid = __syscall(SYS_getpid);
	ret = syscall(SYS_tgkill, pid, tid, sig);
	__restore_sigs(&set);
	return ret;
}
