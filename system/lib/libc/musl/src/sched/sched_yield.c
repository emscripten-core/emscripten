#include <sched.h>
#include "syscall.h"

#if __EMSCRIPTEN__
#include <emscripten/threading.h>
#endif

int sched_yield()
{
#if __EMSCRIPTEN__
	// SharedArrayBuffer and wasm threads do not support explicit yielding.
	// For now we at least call `emscripten_yield` which processes the event queue
	// (along with other essential tasks).
	_emscripten_yield();
	return 0;
#else
	return syscall(SYS_sched_yield);
#endif
}
