#include <stdlib.h>
#include "libc.h"

#define COUNT 32

static void (*funcs[COUNT])(void);
static int count;
static volatile int lock[2];

void __funcs_on_quick_exit()
{
	void (*func)(void);
	LOCK(lock);
	while (count > 0) {
		func = funcs[--count];
		UNLOCK(lock);
		func();
		LOCK(lock);
	}
}

int at_quick_exit(void (*func)(void))
{
	if (count == 32) return -1;
	LOCK(lock);
	funcs[count++] = func;
	UNLOCK(lock);
	return 0;
}
