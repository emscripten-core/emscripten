#include "stdio_impl.h"
#include "pthread_impl.h"

void flockfile(FILE *f)
{
	while (ftrylockfile(f)) {
		int owner = f->lock;
		if (owner) __wait(&f->lock, &f->waiters, owner, 1);
	}
}
