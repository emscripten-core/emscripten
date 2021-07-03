#include <stdlib.h>
#include "syscall.h"

_Noreturn void _Exit(int ec)
{
#ifdef __EMSCRIPTEN__
	__wasi_proc_exit(ec);
#else
	__syscall(SYS_exit_group, ec);
	for (;;) __syscall(SYS_exit, ec);
#endif
}
