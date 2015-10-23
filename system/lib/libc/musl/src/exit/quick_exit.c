#include <stdlib.h>
#include "syscall.h"
#include "atomic.h"
#include "libc.h"

static void dummy() { }
weak_alias(dummy, __funcs_on_quick_exit);

_Noreturn void quick_exit(int code)
{
	static int lock;
	while (a_swap(&lock, 1)) __syscall(SYS_pause);
	__funcs_on_quick_exit();
	_Exit(code);
}
