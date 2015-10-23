#include "stdio_impl.h"
#include "pthread_impl.h"

void funlockfile(FILE *f)
{
	if (!--f->lockcount) __unlockfile(f);
}
