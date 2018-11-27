#include "pthread_impl.h"
#include <threads.h>

_Noreturn void __pthread_exit(void *);

_Noreturn void thrd_exit(int result)
{
	__pthread_exit((void*)(intptr_t)result);
}
