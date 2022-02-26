#include <sched.h>
#include "syscall.h"

#if __EMSCRIPTEN__
#include <emscripten/threading.h>
#endif

int sched_yield()
{
#if __EMSCRIPTEN__
	// SharedArrayBuffer and wasm threads do not support explicit yielding.
	// For now we at least process our event queue so that other threads who
	// are waiting on this one to perform actions can make progesss.
	emscripten_current_thread_process_queued_calls();
	return 0;
#else
	return syscall(SYS_sched_yield);
#endif
}
