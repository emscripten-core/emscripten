#include <sched.h>
#include "syscall.h"

int sched_yield()
{
#if __EMSCRIPTEN__
	// SharedArrayBuffer and wasm threads do not support explicit yielding,
	// but in practice it should happen automatically well enough anyhow, so
	// report success in order to not break apps.
	return 0;
#else
	return syscall(SYS_sched_yield);
#endif
}
