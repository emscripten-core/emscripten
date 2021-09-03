#include "pthread_impl.h"
#include <threads.h>

static void *__pthread_getspecific(pthread_key_t k)
{
	struct pthread *self = __pthread_self();
	// XXX EMSCRIPTEN: self->tsd can be NULL in the case of the
	// main thread where pthread_key_create is not called.
	// See __pthread_key_create for where it get assigned.
	if (!self->tsd) return NULL;
	return self->tsd[k];
}

weak_alias(__pthread_getspecific, pthread_getspecific);
weak_alias(__pthread_getspecific, tss_get);
