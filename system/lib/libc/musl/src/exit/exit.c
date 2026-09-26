#include <stdlib.h>
#include <stdint.h>
#include "libc.h"
#include "pthread_impl.h"
#include "atomic.h"
#include "syscall.h"

static void dummy()
{
}

/* atexit.c and __stdio_exit.c override these. the latter is linked
 * as a consequence of linking either __toread.c or __towrite.c. */
weak_alias(dummy, __funcs_on_exit);
weak_alias(dummy, __stdio_exit);
weak_alias(dummy, _fini);

extern weak hidden void (*const __fini_array_start)(void), (*const __fini_array_end)(void);

static void libc_exit_fini(void)
{
	uintptr_t a = (uintptr_t)&__fini_array_end;
	for (; a>(uintptr_t)&__fini_array_start; a-=sizeof(void(*)()))
		(*(void (**)())(a-sizeof(void(*)())))();
	_fini();
}

weak_alias(libc_exit_fini, __libc_exit_fini);

_Noreturn void exit(int code)
{
	/* Handle potentially concurrent or recursive calls to exit,
	 * whose behaviors have traditionally been undefined by the
	 * standards. Using a custom lock here avoids pulling in lock
	 * machinery and lets us trap recursive calls while supporting
	 * multiple threads contending to be the one to exit(). */
	static volatile int exit_lock[1];
	int tid =  __pthread_self()->tid;
	int prev = a_cas(exit_lock, 0, tid);
	if (prev == tid) a_crash();
	else if (prev) for (;;) __sys_pause();

	__funcs_on_exit();
	__libc_exit_fini();
	__stdio_exit();
	_Exit(code);
}
