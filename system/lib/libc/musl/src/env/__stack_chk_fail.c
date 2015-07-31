#include <string.h>
#include <stdint.h>
#include "pthread_impl.h"
#include "atomic.h"

uintptr_t __stack_chk_guard;

void __init_ssp(void *entropy)
{
	pthread_t self = __pthread_self_init();
	uintptr_t canary;
	if (entropy) memcpy(&canary, entropy, sizeof canary);
	else canary = (uintptr_t)&canary * 1103515245;
	a_cas_l(&__stack_chk_guard, 0, canary);
	self->canary = __stack_chk_guard;
}

void __stack_chk_fail(void)
{
	a_crash();
}
