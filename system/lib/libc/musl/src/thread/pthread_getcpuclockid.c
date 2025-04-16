#include "pthread_impl.h"

int pthread_getcpuclockid(pthread_t t, clockid_t *clockid)
{
#ifdef __EMSCRIPTEN__ // XXX Emscipten per-thread CPU time clocks are not supported
	// pthread API recommends returning this error when "Per-thread CPU time clocks are not supported by the system."
	return ENOENT;
#else
	*clockid = (-t->tid-1)*8U + 6;
	return 0;
#endif
}
