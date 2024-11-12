/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Declarations for internal-only JS library functions.
 *
 * All JS library functions must be declares in one header or anther in order
 * for `tools/gen_sig_info.py` to work.   This file contains declarations for
 * functions that are not declared in any other public or private header.
 */

#include <emscripten/em_macros.h>
#include <emscripten/proxying.h>
#include <emscripten/html5.h>
#include <emscripten/wasm_worker.h>

#include <signal.h>    // for `sighandler_t`
#include <stdbool.h>   // for `bool`
#include <stdint.h>    // for `intptr_t`
#include <sys/types.h> // for `off_t`
#include <time.h>      // for `struct tm`

#ifdef __cplusplus
extern "C" {
#endif

_Noreturn void _abort_js(void);

void setThrew(uintptr_t threw, int value);

// An external JS implementation that is efficient for very large copies, using
// HEAPU8.set()
void _emscripten_memcpy_js(void* __restrict__ dest,
                           const void* __restrict__ src,
                           size_t n) EM_IMPORT(_emscripten_memcpy_js);

void* _emscripten_memcpy_bulkmem(void* __restrict__ dest,
                                 const void* __restrict__ src,
                                 size_t n);
void* _emscripten_memset_bulkmem(void* ptr, char value, size_t n);

void emscripten_notify_memory_growth(size_t memory_index);

time_t _timegm_js(struct tm* tm);
time_t _mktime_js(struct tm* tm);
void _localtime_js(time_t t, struct tm* __restrict__ tm);
void _gmtime_js(time_t t, struct tm* __restrict__ tm);

void _tzset_js(long* timezone, int* daylight, char* std_name, char* dst_name);

const char* emscripten_pc_get_function(uintptr_t pc);
const char* emscripten_pc_get_file(uintptr_t pc);
int emscripten_pc_get_line(uintptr_t pc);
int emscripten_pc_get_column(uintptr_t pc);

void* emscripten_builtin_mmap(
  void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int emscripten_builtin_munmap(void* addr, size_t length);

uintptr_t emscripten_stack_snapshot(void);
uint32_t
emscripten_stack_unwind_buffer(uintptr_t pc, uintptr_t* buffer, uint32_t depth);

bool _emscripten_get_now_is_monotonic(void);

void _emscripten_get_progname(char*, int);

// Not defined in musl, but defined in library.js.  Included here to for
// the benefit of gen_sig_info.py
char* strptime_l(const char* __restrict __s,
                 const char* __restrict __fmt,
                 struct tm* __tp,
                 locale_t __loc);

int _mmap_js(size_t length,
             int prot,
             int flags,
             int fd,
             off_t offset,
             int* allocated,
             void** addr);
int _munmap_js(
  intptr_t addr, size_t length, int prot, int flags, int fd, off_t offset);
int _msync_js(
  intptr_t addr, size_t length, int prot, int flags, int fd, off_t offset);

struct dso;

typedef void (*dlopen_callback_func)(struct dso*, void* user_data);

void* _dlopen_js(struct dso* handle);
void* _dlsym_js(struct dso* handle, const char* symbol, int* sym_index);
void _emscripten_dlopen_js(struct dso* handle,
                           dlopen_callback_func onsuccess,
                           dlopen_callback_func onerror,
                           void* user_data);
void* _dlsym_catchup_js(struct dso* handle, int sym_index);

int _setitimer_js(int which, double timeout);

// Synchronous version of "dlsync_threads".  Called only on the main thread.
// Runs _emscripten_dlsync_self on each of the threads that are running at
// the time of the call.
void _emscripten_dlsync_threads();

// Asynchronous version of "dlsync_threads".  Called only on the main thread.
// Runs _emscripten_dlsync_self on each of the threads that are running at
// the time of the call.  Once this is done the callback is called with the
// given em_proxying_ctx.
void _emscripten_dlsync_threads_async(pthread_t calling_thread,
                                      void (*callback)(em_proxying_ctx*),
                                      em_proxying_ctx* ctx);

#ifdef _GNU_SOURCE
void __call_sighandler(sighandler_t handler, int sig);
#endif

double emscripten_get_now_res(void);

void* emscripten_return_address(int level);

int _emscripten_sanitizer_use_colors(void);
char* _emscripten_sanitizer_get_option(const char* name);

void _emscripten_fs_load_embedded_files(void* ptr);

void _emscripten_throw_longjmp(void);

void _emscripten_runtime_keepalive_clear();

void __handle_stack_overflow(void* addr);

// Internal fetch API
struct emscripten_fetch_t;
void emscripten_start_fetch(struct emscripten_fetch_t* fetch);
size_t _emscripten_fetch_get_response_headers_length(int32_t fetchID);
size_t _emscripten_fetch_get_response_headers(int32_t fetchID, char *dst, size_t dstSizeBytes);
void _emscripten_fetch_free(unsigned int);

EMSCRIPTEN_RESULT _emscripten_set_offscreencanvas_size(const char *target, int width, int height);

// Internal implementation function in JavaScript side that emscripten_create_wasm_worker() calls to
// to perform the wasm worker creation.
emscripten_wasm_worker_t _emscripten_create_wasm_worker(void *stackLowestAddress, uint32_t stackSize);

void __resumeException(void* exn);
void __cxa_call_unexpected(void* exn);
void llvm_eh_typeid_for(void* exn);

uint32_t _emscripten_lookup_name(const char *name);

int _emscripten_system(const char *command);

#ifdef __cplusplus
}
#endif
