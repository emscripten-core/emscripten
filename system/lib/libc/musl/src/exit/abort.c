#include <stdlib.h>
#include <signal.h>
#include "syscall.h"
#include "pthread_impl.h"
#include "atomic.h"

_Noreturn void abort(void)
{
	raise(SIGABRT);
	__block_all_sigs(0);
	a_crash();
	raise(SIGKILL);
	_Exit(127);
}
