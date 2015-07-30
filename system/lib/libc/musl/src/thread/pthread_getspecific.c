#include "pthread_impl.h"

void *pthread_getspecific(pthread_key_t k)
{
	struct pthread *self = pthread_self();
	return self->tsd[k];
}
