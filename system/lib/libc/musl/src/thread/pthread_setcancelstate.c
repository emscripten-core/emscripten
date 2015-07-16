#include "pthread_impl.h"

int pthread_setcancelstate(int new, int *old)
{
	if (new > 1U) return EINVAL;
	if (libc.main_thread) {
		struct pthread *self = __pthread_self();
		if (old) *old = self->canceldisable;
		self->canceldisable = new;
	} else {
		if (old) *old = libc.canceldisable;
		libc.canceldisable = new;
	}
	return 0;
}
