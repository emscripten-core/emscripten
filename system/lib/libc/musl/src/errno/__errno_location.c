#include "pthread_impl.h"

int *__errno_location(void)
{
	return &__pthread_self()->errno_val;
}
