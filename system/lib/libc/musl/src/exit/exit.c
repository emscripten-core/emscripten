#include <stdlib.h>
#include <stdint.h>
#include "libc.h"
#include "atomic.h"
#include "syscall.h"

static void dummy()
{
}

/* atexit.c and __stdio_exit.c override these. the latter is linked
 * as a consequence of linking either __toread.c or __towrite.c. */
weak_alias(dummy, __funcs_on_exit);
weak_alias(dummy, __stdio_exit);

#ifndef SHARED
weak_alias(dummy, _fini);
extern void (*const __fini_array_start)() __attribute__((weak));
extern void (*const __fini_array_end)() __attribute__((weak));
#endif

_Noreturn void exit(int code)
{
	static int lock;

	/* If more than one thread calls exit, hang until _Exit ends it all */
	while (a_swap(&lock, 1)) __syscall(SYS_pause);

	__funcs_on_exit();

#ifndef SHARED
	uintptr_t a = (uintptr_t)&__fini_array_end;
	for (; a>(uintptr_t)&__fini_array_start; a-=sizeof(void(*)()))
		(*(void (**)())(a-sizeof(void(*)())))();
	_fini();
#endif

	__stdio_exit();

	_Exit(code);
	for(;;);
}
