#include <time.h>
#include "syscall.h"

int clock_getres(clockid_t clk, struct timespec *ts)
{
#ifdef __EMSCRIPTEN__
	// See https://github.com/bytecodealliance/wasmtime/issues/374
	if (clk > __WASI_CLOCKID_THREAD_CPUTIME_ID || clk < 0) {
		errno = EINVAL;
		return -1;
	}
	__wasi_timestamp_t res;
	__wasi_errno_t error = __wasi_clock_res_get(clk, &res);
	if (error != __WASI_ERRNO_SUCCESS) {
		return __wasi_syscall_ret(error);
	}
	*ts = __wasi_timestamp_to_timespec(res);
	return 0;
#else
#ifdef SYS_clock_getres_time64
	/* On a 32-bit arch, use the old syscall if it exists. */
	if (SYS_clock_getres != SYS_clock_getres_time64) {
		long ts32[2];
		int r = __syscall(SYS_clock_getres, clk, ts32);
		if (!r && ts) {
			ts->tv_sec = ts32[0];
			ts->tv_nsec = ts32[1];
		}
		return __syscall_ret(r);
	}
#endif
	/* If reaching this point, it's a 64-bit arch or time64-only
	 * 32-bit arch and we can get result directly into timespec. */
	return syscall(SYS_clock_getres, clk, ts);
#endif
}
