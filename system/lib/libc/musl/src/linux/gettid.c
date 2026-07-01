#define _GNU_SOURCE
#include <unistd.h>
#include "pthread_impl.h"

#ifdef __EMSCRIPTEN__
weak int emscripten_wasm_worker_self_id();
#endif

#if defined(__EMSCRIPTEN_WASM_WORKERS__) && defined(__EMSCRIPTEN_PTHREADS__)
#error "this file should be compiled with either wasm workers or pthreads but not both"
#endif

pid_t gettid(void)
{
#ifdef __EMSCRIPTEN_WASM_WORKERS__
	// Offset the worker ID by 1 so we never return 0 from this function.
	// Strangly we cannot assume the existence of emscripten_wasm_worker_self_id
	// here because libc-ww is also used for `-sSHARED_MEMORY` builds (without
	// libwasm_workers linked in.
	if (emscripten_wasm_worker_self_id) {
		return emscripten_wasm_worker_self_id() + 1;
	} else {
		return 42;
	}
#else
#if defined(__EMSCRIPTEN_PTHREADS__)
	// The pthread-variant of libc can also be used alongside wasm workers.
	// We detect that via a weak reference to the self_id function.
	if (emscripten_wasm_worker_self_id) {
		pid_t rtn = emscripten_wasm_worker_self_id();
		if (rtn) return rtn;
	}
#endif
	return __pthread_self()->tid;
#endif
}
