#include <threads.h>

int __pthread_once(once_flag *, void (*)(void));

void call_once(once_flag *flag, void (*func)(void))
{
	__pthread_once(flag, func);
}
