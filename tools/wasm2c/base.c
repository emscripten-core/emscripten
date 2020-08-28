/*
 * Base of all support for wasm2c code.
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ssize_t detection: usually stdint provides it, but not on windows apparently
#ifdef _WIN32
#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else // _MSC_VER
#ifdef _WIN64
typedef signed long long ssize_t;
#else // _WIN64
typedef signed long ssize_t;
#endif // _WIN64
#endif // _MSC_VER
#endif // _WIN32

#include "wasm-rt.h"
#include "wasm-rt-impl.h"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#define MEMACCESS(addr) ((void*)&Z_memory->data[addr])

#undef MEMCHECK
#define MEMCHECK(a, t)  \
  if (UNLIKELY((a) + sizeof(t) > Z_memory->size)) TRAP(OOB)

#undef DEFINE_LOAD
#define DEFINE_LOAD(name, t1, t2, t3)              \
  static inline t3 name(u64 addr) {   \
    MEMCHECK(addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, MEMACCESS(addr), sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#undef DEFINE_STORE
#define DEFINE_STORE(name, t1, t2)                           \
  static inline void name(u64 addr, t2 value) { \
    MEMCHECK(addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(MEMACCESS(addr), &wrapped, sizeof(t1));          \
  }

DEFINE_LOAD(wasm_i32_load, u32, u32, u32);
DEFINE_LOAD(wasm_i64_load, u64, u64, u64);
DEFINE_LOAD(wasm_f32_load, f32, f32, f32);
DEFINE_LOAD(wasm_f64_load, f64, f64, f64);
DEFINE_LOAD(wasm_i32_load8_s, s8, s32, u32);
DEFINE_LOAD(wasm_i64_load8_s, s8, s64, u64);
DEFINE_LOAD(wasm_i32_load8_u, u8, u32, u32);
DEFINE_LOAD(wasm_i64_load8_u, u8, u64, u64);
DEFINE_LOAD(wasm_i32_load16_s, s16, s32, u32);
DEFINE_LOAD(wasm_i64_load16_s, s16, s64, u64);
DEFINE_LOAD(wasm_i32_load16_u, u16, u32, u32);
DEFINE_LOAD(wasm_i64_load16_u, u16, u64, u64);
DEFINE_LOAD(wasm_i64_load32_s, s32, s64, u64);
DEFINE_LOAD(wasm_i64_load32_u, u32, u64, u64);
DEFINE_STORE(wasm_i32_store, u32, u32);
DEFINE_STORE(wasm_i64_store, u64, u64);
DEFINE_STORE(wasm_f32_store, f32, f32);
DEFINE_STORE(wasm_f64_store, f64, f64);
DEFINE_STORE(wasm_i32_store8, u8, u32);
DEFINE_STORE(wasm_i32_store16, u16, u32);
DEFINE_STORE(wasm_i64_store8, u8, u64);
DEFINE_STORE(wasm_i64_store16, u16, u64);
DEFINE_STORE(wasm_i64_store32, u32, u64);

// Imports

#ifdef VERBOSE_LOGGING
#define VERBOSE_LOG(...) { printf(__VA_ARGS__); }
#else
#define VERBOSE_LOG(...)
#endif

#define IMPORT_IMPL(ret, name, params, body) \
static ret _##name params { \
  VERBOSE_LOG("[import: " #name "]\n"); \
  body \
} \
ret (*name) params = _##name;

#define STUB_IMPORT_IMPL(ret, name, params, returncode) IMPORT_IMPL(ret, name, params, { return returncode; });

// Generic abort method for a runtime error in the runtime.

static void abort_with_message(const char* message) {
  fprintf(stderr, "%s\n", message);
  abort();
}

// Maintain a stack of setjmps, each jump taking us back to the last invoke.

#define MAX_SETJMP_STACK 1024

static jmp_buf setjmp_stack[MAX_SETJMP_STACK];

static u32 next_setjmp = 0;

// Declare an export that may be needed and may not be. For example if longjmp
// is included then we need setThrew, but we must declare setThrew so that
// the C compiler can build us without error if longjmp is not actually used.

#define DECLARE_WEAK_EXPORT(ret, name, args) \
__attribute__((weak)) \
ret (*WASM_RT_ADD_PREFIX(name)) args = NULL;

DECLARE_WEAK_EXPORT(void, Z_setThrewZ_vii, (u32, u32));

IMPORT_IMPL(void, Z_envZ_emscripten_longjmpZ_vii, (u32 buf, u32 value), {
  if (next_setjmp == 0) {
    abort_with_message("longjmp without setjmp");
  }
  Z_setThrewZ_vii(buf, value ? value : 1);
  longjmp(setjmp_stack[next_setjmp - 1], 1);
});

IMPORT_IMPL(void, Z_envZ_emscripten_notify_memory_growthZ_vi, (u32 size), {});

static u32 tempRet0 = 0;

IMPORT_IMPL(u32, Z_envZ_getTempRet0Z_iv, (), {
  return tempRet0;
});

IMPORT_IMPL(void, Z_envZ_setTempRet0Z_vi, (u32 x), {
  tempRet0 = x;
});

// Shared OS support in both sandboxed and unsandboxed mode

#define WASI_DEFAULT_ERROR 63 /* __WASI_ERRNO_PERM */
#define WASI_EINVAL 28

// Syscalls return a negative error code
#define EM_EACCES -2

STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_fdstat_getZ_iii, (u32 a, u32 b), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_syncZ_ii, (u32 a), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_envZ_dlopenZ_iii, (u32 a, u32 b), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlcloseZ_ii, (u32 a), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlsymZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_dlerrorZ_iv, (), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_signalZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_systemZ_ii, (u32 a), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_utimesZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_rmdirZ_ii, (u32 a), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_renameZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_lstat64Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup3Z_iiii, (u32 a, u32 b, u32 c), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup2Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_getcwdZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_ftruncate64Z_iiiii, (u32 a, u32 b, u32 c, u32 d), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_initZ_ii, (u32 a), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_settypeZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_destroyZ_ii, (u32 a), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_createZ_iiiii, (u32 a, u32 b, u32 c, u32 d), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_joinZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ___cxa_thread_atexitZ_iiii, (u32 a, u32 b, u32 c), -1);
