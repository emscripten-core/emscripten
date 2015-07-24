#include <stdlib.h>
#include <signal.h>
#include "syscall.h"

_Noreturn void abort(void)
{
	raise(SIGABRT);
	raise(SIGKILL);
	for (;;);
}
