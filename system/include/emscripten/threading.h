#ifndef __emscripten_threading_h__
#define __emscripten_threading_h__

#include <inttypes.h>
#include <pthread.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns true if the current browser is able to spawn threads with pthread_create(), and the compiled page was built with
// threading support enabled. If this returns 0, calls to pthread_create() will fail with return code EAGAIN.
int emscripten_has_threading_support(void);

// Returns the number of logical cores on the system.
int emscripten_num_logical_cores(void);

// Configures the number of logical cores on the system. This can be called at startup
// to specify the number of cores emscripten_num_logical_cores() reports. The
// Emscripten system itself does not use this value internally anywhere, it is just
// a hint to help developers have a single access point 'emscripten_num_logical_cores()'
// to query the number of cores in the system.
void emscripten_force_num_logical_cores(int cores);

// Atomically stores the given value to the memory location, and returns the value that was there prior to the store.
uint8_t emscripten_atomic_exchange_u8(void/*uint8_t*/ *addr, uint8_t newVal);
uint16_t emscripten_atomic_exchange_u16(void/*uint16_t*/ *addr, uint16_t newVal);
uint32_t emscripten_atomic_exchange_u32(void/*uint32_t*/ *addr, uint32_t newVal);
uint64_t emscripten_atomic_exchange_u64(void/*uint64_t*/ *addr, uint64_t newVal); // Emulated with locks, very slow!!

// CAS returns the *old* value that was in the memory location before the operation took place.
// That is, if the return value when calling this function equals to 'oldVal', then the operation succeeded,
// otherwise it was ignored.
uint8_t emscripten_atomic_cas_u8(void/*uint8_t*/ *addr, uint8_t oldVal, uint8_t newVal);
uint16_t emscripten_atomic_cas_u16(void/*uint16_t*/ *addr, uint16_t oldVal, uint16_t newVal);
uint32_t emscripten_atomic_cas_u32(void/*uint32_t*/ *addr, uint32_t oldVal, uint32_t newVal);
uint64_t emscripten_atomic_cas_u64(void/*uint64_t*/ *addr, uint64_t oldVal, uint64_t newVal); // Emulated with locks, very slow!!

uint8_t emscripten_atomic_load_u8(const void/*uint8_t*/ *addr);
uint16_t emscripten_atomic_load_u16(const void/*uint16_t*/ *addr);
uint32_t emscripten_atomic_load_u32(const void/*uint32_t*/ *addr);
float emscripten_atomic_load_f32(const void/*float*/ *addr);
uint64_t emscripten_atomic_load_u64(const void/*uint64_t*/ *addr); // Emulated with locks, very slow!!
double emscripten_atomic_load_f64(const void/*double*/ *addr); // Emulated with locks, very slow!!

// Returns the value that was stored (i.e. 'val')
uint8_t emscripten_atomic_store_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_store_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_store_u32(void/*uint32_t*/ *addr, uint32_t val);
float emscripten_atomic_store_f32(void/*float*/ *addr, float val);
uint64_t emscripten_atomic_store_u64(void/*uint64_t*/ *addr, uint64_t val); // Emulated with locks, very slow!!
double emscripten_atomic_store_f64(void/*double*/ *addr, double val); // Emulated with locks, very slow!!

void emscripten_atomic_fence(void);

// Each of the functions below (add, sub, and, or, xor) returns the value that was stored to memory after the operation occurred.
uint8_t emscripten_atomic_add_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_add_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_add_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_add_u64(void/*uint64_t*/ *addr, uint64_t val); // Emulated with locks, very slow!!

uint8_t emscripten_atomic_sub_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_sub_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_sub_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_sub_u64(void/*uint64_t*/ *addr, uint64_t val); // Emulated with locks, very slow!!

uint8_t emscripten_atomic_and_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_and_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_and_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_and_u64(void/*uint64_t*/ *addr, uint64_t val); // Emulated with locks, very slow!!

uint8_t emscripten_atomic_or_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_or_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_or_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_or_u64(void/*uint64_t*/ *addr, uint64_t val); // Emulated with locks, very slow!!

uint8_t emscripten_atomic_xor_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_xor_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_xor_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_xor_u64(void/*uint64_t*/ *addr, uint64_t val); // Emulated with locks, very slow!!

int emscripten_futex_wait(volatile void/*uint32_t*/ *addr, uint32_t val, double maxWaitMilliseconds);
int emscripten_futex_wake(volatile void/*uint32_t*/ *addr, int count);
int emscripten_futex_wake_or_requeue(volatile void/*uint32_t*/ *addr, int count, volatile void/*uint32_t*/ *addr2, int cmpValue);

typedef union em_variant_val
{
  int i;
  float f;
  double d;
  void *vp;
  char *cp;
} em_variant_val;

#define EM_QUEUED_CALL_MAX_ARGS 8
typedef struct em_queued_call
{
  int functionEnum;
  void *functionPtr;
  int operationDone;
  em_variant_val args[EM_QUEUED_CALL_MAX_ARGS];
  em_variant_val returnValue;

  // If true, the caller has "detached" itself from this call
  // object and the Emscripten main runtime thread should free up
  // this em_queued_call object after it has been executed. If
  // false, the caller is in control of the memory.
  int calleeDelete;
} em_queued_call;

void emscripten_sync_run_in_main_thread(em_queued_call *call);
void *emscripten_sync_run_in_main_thread_0(int function);
void *emscripten_sync_run_in_main_thread_1(int function, void *arg1);
void *emscripten_sync_run_in_main_thread_2(int function, void *arg1, void *arg2);
void *emscripten_sync_run_in_main_thread_3(int function, void *arg1, void *arg2, void *arg3);
void *emscripten_sync_run_in_main_thread_7(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7);

typedef void (*em_func_v)(void);
typedef void (*em_func_vi)(int);
typedef void (*em_func_vii)(int, int);
typedef void (*em_func_viii)(int, int, int);
typedef int (*em_func_i)(void);
typedef int (*em_func_ii)(int);
typedef int (*em_func_iii)(int, int);
typedef int (*em_func_iiii)(int, int, int);

#define EM_FUNC_SIGNATURE int

#define EM_FUNC_SIG_V 1024
#define EM_FUNC_SIG_VI (1024 + 1)
#define EM_FUNC_SIG_VII (1024 + 2)
#define EM_FUNC_SIG_VIII (1024 + 3)
#define EM_FUNC_SIG_I 2048
#define EM_FUNC_SIG_II (2048 + 1)
#define EM_FUNC_SIG_III (2048 + 2)
#define EM_FUNC_SIG_IIII (2048 + 3)

#define EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(x) ((x) & 1023)

// Runs the given function synchronously on the main Emscripten runtime thread.
// If this thread is the main thread, the operation is immediately performed, and the result is returned.
// If the current thread is not the main Emscripten runtime thread (but a pthread), the function
// will be proxied to be called by the main thread.
//  - Calling emscripten_sync_* functions requires that the application was compiled with pthreads
//    support enabled (-s USE_PTHREADS=1/2) and that the browser supports SharedArrayBuffer specification.
int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...);

// The 'async' variant of the run_in_main_thread functions are otherwise the same as the synchronous ones,
// except that the operation is performed in a fire and forget manner. The call is placed to the command
// queue of the main Emscripten runtime thread, but its completion is not waited for. As a result, if
// the function did have a return value, the return value is not received.
//  - Note that multiple asynchronous commands from a single pthread/Worker are guaranteed to be executed
//    on the main thread in the program order they were called in.
void emscripten_async_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...);

// The 'async_waitable' variant of the run_in_main_runtime_thread functions run like the 'async' variants, except
// that while the operation starts off asynchronously, the result is then later waited upon to receive
// the return value.
//  - The object returned by this function call is dynamically allocated, and should be freed up via a call
//    to emscripten_async_waitable_close() after the wait has been performed.
em_queued_call *emscripten_async_waitable_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...);

// Since we can't validate the function pointer type, allow implicit casting of functions to void* without complaining.
#define emscripten_sync_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_sync_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)
#define emscripten_async_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_async_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)
#define emscripten_async_waitable_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_async_waitable_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)

EMSCRIPTEN_RESULT emscripten_wait_for_call_v(em_queued_call *call, double timeoutMSecs);
EMSCRIPTEN_RESULT emscripten_wait_for_call_i(em_queued_call *call, double timeoutMSecs, int *outResult);

void emscripten_async_waitable_close(em_queued_call *call);

// Returns 1 if the current thread is the thread that hosts the Emscripten runtime.
int emscripten_is_main_runtime_thread(void);

// Returns 1 if the current thread is the main browser thread.
int emscripten_is_main_browser_thread(void);

// A temporary workaround to issue https://github.com/kripken/emscripten/issues/3495:
// Call this in the body of all lock-free atomic (cas) loops that the main thread might enter
// which don't otherwise call to any pthread api calls (mutexes) or C runtime functions
// that are considered cancellation points.
void emscripten_main_thread_process_queued_calls(void);

// Direct syscall access, second argument is a varargs pointer. used in proxying
int emscripten_syscall(int, void*);

#define EM_THREAD_STATUS int
#define EM_THREAD_STATUS_NOTSTARTED 0
#define EM_THREAD_STATUS_RUNNING    1
#define EM_THREAD_STATUS_SLEEPING   2 // Performing an unconditional sleep (usleep, etc.)
#define EM_THREAD_STATUS_WAITFUTEX  3 // Waiting for an explicit low-level futex (emscripten_futex_wait)
#define EM_THREAD_STATUS_WAITMUTEX  4 // Waiting for a pthread_mutex_t
#define EM_THREAD_STATUS_WAITPROXY  5 // Waiting for a proxied operation to finish.
#define EM_THREAD_STATUS_FINISHED   6
#define EM_THREAD_STATUS_NUMFIELDS  7

// Sets the profiler status of the calling thread. This is a no-op if thread profiling is not active.
// This is an internal function and generally not intended for user code.
// When thread profiler is not enabled (not building with --threadprofiling), this is a no-op.
void emscripten_set_current_thread_status(EM_THREAD_STATUS newStatus);

// Sets the profiler status of the calling thread, but only if it was in the expected status beforehand.
// This is an internal function and generally not intended for user code.
// When thread profiler is not enabled (not building with --threadprofiling), this is a no-op.
void emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS expectedStatus, EM_THREAD_STATUS newStatus);

// Sets the name of the given thread. Pass pthread_self() as the thread ID to set the name of the calling thread.
// The name parameter is a UTF-8 encoded string which is truncated to 32 bytes.
// When thread profiler is not enabled (not building with --threadprofiling), this is a no-op.
void emscripten_set_thread_name(pthread_t threadId, const char *name);

// Gets the stored pointer to a string representing the canvases to transfer to the created thread.
int emscripten_pthread_attr_gettransferredcanvases(const pthread_attr_t *a, const char **str);

// Specifies a comma-delimited list of canvas DOM element IDs to transfer to the thread to be created.
// Note: this pointer is weakly stored (not copied) to the given pthread_attr_t, so must be held alive until
// pthread_create() has been called. If 0 or "", no canvases are transferred. The special value "#canvas" denotes
// the element stored in Module.canvas.
int emscripten_pthread_attr_settransferredcanvases(pthread_attr_t *a, const char *str);

struct thread_profiler_block
{
	// One of THREAD_STATUS_*
	int threadStatus;
	// Wallclock time denoting when the current thread state was entered in.
	double currentStatusStartTime;
	// Accumulated duration times denoting how much time has been spent in each state, in msecs.
	double timeSpentInStatus[EM_THREAD_STATUS_NUMFIELDS];
	// A human-readable name for this thread.
	char name[32];
};

#define EM_PROXIED_UTIME 12
#define EM_PROXIED_UTIMES 13
#define EM_PROXIED_CHROOT 37
#define EM_PROXIED_FPATHCONF 46
#define EM_PROXIED_CONFSTR 68
#define EM_PROXIED_SYSCONF 72
#define EM_PROXIED_ATEXIT 110
#define EM_PROXIED_GETENV 111
#define EM_PROXIED_CLEARENV 112
#define EM_PROXIED_SETENV 113
#define EM_PROXIED_UNSETENV 114
#define EM_PROXIED_PUTENV 115
#define EM_PROXIED_TZSET 119
#define EM_PROXIED_PTHREAD_CREATE 137
#define EM_PROXIED_SYSCALL 138

#ifdef __cplusplus
}
#endif

#endif
