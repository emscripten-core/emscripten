#include "pthread_impl.h"

int pthread_rwlock_init(pthread_rwlock_t *restrict rw, const pthread_rwlockattr_t *restrict a)
{
	*rw = (pthread_rwlock_t){0};
	if (a) {
	}
	return 0;
}
