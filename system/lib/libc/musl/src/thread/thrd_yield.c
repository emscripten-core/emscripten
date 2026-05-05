#include <threads.h>
#include "syscall.h"

void thrd_yield()
{
#ifndef __EMSCRIPTEN__
	__syscall(SYS_sched_yield);
#endif
}
