#include <threads.h>

int __private_cond_signal(cnd_t *, int);

int cnd_signal(cnd_t *c)
{
	/* This internal function never fails, and always returns zero,
	 * which matches the value thrd_success is defined with. */
	return __private_cond_signal(c, 1);
}
