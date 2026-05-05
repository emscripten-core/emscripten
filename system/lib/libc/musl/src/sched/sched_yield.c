#include <sched.h>
#include "syscall.h"

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include "threading_internal.h"
#endif

int sched_yield()
{
#if __EMSCRIPTEN__
	// SharedArrayBuffer and wasm threads do not support explicit yielding.
	// For now we at least call `emscripten_yield` which processes the event queue
	// (along with other essential tasks).
	_emscripten_yield(emscripten_get_now());
	return 0;
#else
	return syscall(SYS_sched_yield);
#endif
}
