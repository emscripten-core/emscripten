/*
 * A main file to run wasm2c code. This implements various wasi and emscripten
 * syscalls, and allows direct/unsandboxed file access TODO add options
 */

#define __USE_GNU // for O_PATH

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "wasm-rt.h"
#include "wasm-rt-impl.h"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#define MEMACCESS(addr) ((void*)&Z_memory->data[addr])

#define MEMCHECK(a, t)  \
  if (UNLIKELY((a) + sizeof(t) > Z_memory->size)) TRAP(OOB)

#define DEFINE_LOAD(name, t1, t2, t3)              \
  static inline t3 name(u64 addr) {   \
    MEMCHECK(addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, MEMACCESS(addr), sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#define DEFINE_STORE(name, t1, t2)                           \
  static inline void name(u64 addr, t2 value) { \
    MEMCHECK(addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(MEMACCESS(addr), &wrapped, sizeof(t1));          \
  }

DEFINE_LOAD(i32_load, u32, u32, u32);
DEFINE_LOAD(i64_load, u64, u64, u64);
DEFINE_LOAD(f32_load, f32, f32, f32);
DEFINE_LOAD(f64_load, f64, f64, f64);
DEFINE_LOAD(i32_load8_s, s8, s32, u32);
DEFINE_LOAD(i64_load8_s, s8, s64, u64);
DEFINE_LOAD(i32_load8_u, u8, u32, u32);
DEFINE_LOAD(i64_load8_u, u8, u64, u64);
DEFINE_LOAD(i32_load16_s, s16, s32, u32);
DEFINE_LOAD(i64_load16_s, s16, s64, u64);
DEFINE_LOAD(i32_load16_u, u16, u32, u32);
DEFINE_LOAD(i64_load16_u, u16, u64, u64);
DEFINE_LOAD(i64_load32_s, s32, s64, u64);
DEFINE_LOAD(i64_load32_u, u32, u64, u64);
DEFINE_STORE(i32_store, u32, u32);
DEFINE_STORE(i64_store, u64, u64);
DEFINE_STORE(f32_store, f32, f32);
DEFINE_STORE(f64_store, f64, f64);
DEFINE_STORE(i32_store8, u8, u32);
DEFINE_STORE(i32_store16, u16, u32);
DEFINE_STORE(i64_store8, u8, u64);
DEFINE_STORE(i64_store16, u16, u64);
DEFINE_STORE(i64_store32, u32, u64);

// Imports

#ifdef VERBOSE_LOGGING
#define VERBOSE_LOG(...) { printf(__VA_ARGS__); }
#else
#define VERBOSE_LOG(...)
#endif

#define IMPORT_IMPL(ret, name, params, body) \
ret _##name params { \
  VERBOSE_LOG("[import: " #name "]\n"); \
  body \
} \
ret (*name) params = _##name;

#define STUB_IMPORT_IMPL(ret, name, params, returncode) IMPORT_IMPL(ret, name, params, { return returncode; });

#define WASI_DEFAULT_ERROR 63 /* __WASI_ERRNO_PERM */

IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (u32 x), {
  exit(x);
});

#define MAX_FDS 1024

static int wasm_fd_to_native[MAX_FDS];

static u32 next_wasm_fd;

static void init_fds() {
  wasm_fd_to_native[0] = STDIN_FILENO;
  wasm_fd_to_native[1] = STDOUT_FILENO;
  wasm_fd_to_native[2] = STDERR_FILENO;
  next_wasm_fd = 3;
}

void abort_with_message(const char* message) {
  fprintf(stderr, "%s\n", message);
  abort();
}

static u32 get_or_allocate_wasm_fd(int nfd) {
  // If the native fd is already mapped, return the same wasm fd for it.
  for (int i = 0; i < next_wasm_fd; i++) {
    if (wasm_fd_to_native[i] == nfd) {
      return i;
    }
  }
  if (next_wasm_fd >= MAX_FDS) {
    abort_with_message("ran out of fds");
  }
  u32 fd = next_wasm_fd;
  wasm_fd_to_native[fd] = nfd;
  next_wasm_fd++;
  return fd;
}

static int get_native_fd(u32 fd) {
  if (fd >= MAX_FDS || fd >= next_wasm_fd) {
    return -1;
  }
  return wasm_fd_to_native[fd];
}

IMPORT_IMPL(u32, Z_envZ___sys_openZ_iiii, (u32 path, u32 flags, u32 varargs), {
  VERBOSE_LOG("  open: %s %d %d\n", MEMACCESS(path), flags, i32_load(varargs));
  int nfd = open(MEMACCESS(path), flags, i32_load(varargs));
  VERBOSE_LOG("    => native %d\n", nfd);
  if (nfd >= 0) {
    u32 fd = get_or_allocate_wasm_fd(nfd);
    VERBOSE_LOG("      => wasm %d\n", fd);
    return fd;
  }
  return -1;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  fd_write wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = i32_load(iov + i * 8);
    u32 len = i32_load(iov + i * 8 + 4);
    VERBOSE_LOG("    chunk %d %d\n", ptr, len);
    ssize_t result;
    // Use stdio for stdout/stderr to avoid mixing a low-level write() with
    // other logging code, which can change the order from the expected.
    if (nfd == STDOUT_FILENO) {
      result = fwrite(MEMACCESS(ptr), 1, len, stdout);
    } else if (nfd == STDERR_FILENO) {
      result = fwrite(MEMACCESS(ptr), 1, len, stderr);
    } else {
      result = write(nfd, MEMACCESS(ptr), len);
    }
    if (result < 0) {
      VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
      return WASI_DEFAULT_ERROR;
    }
    if (result != len) {
      VERBOSE_LOG("    amount error, %ld %d\n", result, len);
      return WASI_DEFAULT_ERROR;
    }
    num += len;
  }
  VERBOSE_LOG("    success: %d\n", num);
  i32_store(pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_readZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  fd_read wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = i32_load(iov + i * 8);
    u32 len = i32_load(iov + i * 8 + 4);
    VERBOSE_LOG("    chunk %d %d\n", ptr, len);
    ssize_t result = read(nfd, MEMACCESS(ptr), len);
    if (result < 0) {
      VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
      return WASI_DEFAULT_ERROR;
    }
    num += result;
    if (result != len) {
      break; // nothing more to read
    }
  }
  VERBOSE_LOG("    success: %d\n", num);
  i32_store(pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_closeZ_ii, (u32 fd), {
  // TODO full file support
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  close wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  close(nfd);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_getZ_iii, (u32 pcount, u32 pbuf_size), {
  // TODO: connect to actual env?
  i32_store(pcount, 0);
  i32_store(pbuf_size, 0);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_getZ_iii, (u32 __environ, u32 environ_buf), {
  // TODO: connect to actual env?
  return 0;
});

static int whence_to_native(u32 whence) {
  if (whence == 0) return SEEK_SET;
  if (whence == 1) return SEEK_CUR;
  if (whence == 2) return SEEK_END;
  return -1;
}

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iijii, (u32 fd, u64 offset, u32 whence, u32 new_offset), {
  int nfd = get_native_fd(fd);
  int nwhence = whence_to_native(whence);
  VERBOSE_LOG("  seek %d (=> native %d) %ld %d (=> %d) %d\n", fd, nfd, offset, whence, nwhence, new_offset);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  off_t off = lseek(nfd, offset, nwhence);
  VERBOSE_LOG("    off: %ld\n", off);
  if (off == (off_t)-1) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return WASI_DEFAULT_ERROR;
  }
  i64_store(new_offset, off);
  return 0;
});
IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iiiiii, (u32 a, u32 b, u32 c, u32 d, u32 e), {
  return Z_wasi_snapshot_preview1Z_fd_seekZ_iijii(a, b + (((u64)c) << 32), d, e);
});
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_fdstat_getZ_iii, (u32 a, u32 b), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_syncZ_ii, (u32 a), WASI_DEFAULT_ERROR);

// TODO: set errno in wasm for everything

STUB_IMPORT_IMPL(u32, Z_envZ_dlopenZ_iii, (u32 a, u32 b), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlcloseZ_ii, (u32 a), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlsymZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_dlerrorZ_iv, (), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_signalZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_systemZ_ii, (u32 a), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_utimesZ_iii, (u32 a, u32 b), -1);

// Syscalls return a negative error code
#define EM_EACCES -2

IMPORT_IMPL(u32, Z_envZ___sys_unlinkZ_ii, (u32 path), {
  VERBOSE_LOG("  unlink %s\n", MEMACCESS(path));
  if (unlink(MEMACCESS(path))) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return 0;
});
STUB_IMPORT_IMPL(u32, Z_envZ___sys_rmdirZ_ii, (u32 a), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_renameZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_lstat64Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup3Z_iiii, (u32 a, u32 b, u32 c), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup2Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_getcwdZ_iii, (u32 a, u32 b), EM_EACCES);

static u32 do_stat(int nfd, u32 buf) {
  struct stat nbuf;
  if (fstat(nfd, &nbuf)) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  VERBOSE_LOG("    success, size=%ld\n", nbuf.st_size);
  i32_store(buf + 0, nbuf.st_dev);
  i32_store(buf + 4, 0);
  i32_store(buf + 8, nbuf.st_ino);
  i32_store(buf + 12, nbuf.st_mode);
  i32_store(buf + 16, nbuf.st_nlink);
  i32_store(buf + 20, nbuf.st_uid);
  i32_store(buf + 24, nbuf.st_gid);
  i32_store(buf + 28, nbuf.st_rdev);
  i32_store(buf + 32, 0);
  i64_store(buf + 40, nbuf.st_size);
  i32_store(buf + 48, nbuf.st_blksize);
  i32_store(buf + 52, nbuf.st_blocks);
  i32_store(buf + 56, nbuf.st_atim.tv_sec);
  i32_store(buf + 60, nbuf.st_atim.tv_nsec);
  i32_store(buf + 64, nbuf.st_mtim.tv_sec);
  i32_store(buf + 68, nbuf.st_mtim.tv_nsec);
  i32_store(buf + 72, nbuf.st_ctim.tv_sec);
  i32_store(buf + 76, nbuf.st_ctim.tv_nsec);
  i64_store(buf + 80, nbuf.st_ino);
  return 0;
}

IMPORT_IMPL(u32, Z_envZ___sys_fstat64Z_iii, (u32 fd, u32 buf), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  fstat64 %d (=> %d) %d\n", fd, nfd, buf);
  if (nfd < 0) {
    return EM_EACCES;
  }
  return do_stat(nfd, buf);
});

IMPORT_IMPL(u32, Z_envZ___sys_stat64Z_iii, (u32 path, u32 buf), {
  VERBOSE_LOG("  stat64: %s\n", MEMACCESS(path));
  int nfd = open(MEMACCESS(path), O_PATH);
  if (nfd < 0) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return do_stat(nfd, buf);
});

STUB_IMPORT_IMPL(u32, Z_envZ___sys_ftruncate64Z_iiiii, (u32 a, u32 b, u32 c, u32 d), EM_EACCES);
IMPORT_IMPL(u32, Z_envZ___sys_readZ_iiii, (u32 fd, u32 buf, u32 count), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  read %d (=> %d) %d %d\n", fd, nfd, buf, count);
  if (nfd < 0) {
    VERBOSE_LOG("    bad fd\n");
    return EM_EACCES;
  }
  ssize_t ret = read(nfd, MEMACCESS(buf), count);
  VERBOSE_LOG("    native read: %ld\n", ret);
  if (ret < 0) {
    VERBOSE_LOG("    read error %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return ret;
});

IMPORT_IMPL(u32, Z_envZ___sys_accessZ_iii, (u32 pathname, u32 mode), {
  VERBOSE_LOG("  access: %s 0x%x\n", MEMACCESS(pathname), mode);
  // TODO: sandboxing, convert mode
  int result = access(MEMACCESS(pathname), mode);
  if (result < 0) {
    VERBOSE_LOG("    access error: %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return 0;
});

STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_initZ_ii, (u32 a), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_settypeZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_destroyZ_ii, (u32 a), 0);

static int main_argc;
static char** main_argv;

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_args_sizes_getZ_iii, (u32 pargc, u32 pargv_buf_size), {
  i32_store(pargc, main_argc);
  u32 buf_size = 0;
  for (u32 i = 0; i < main_argc; i++) {
    buf_size += strlen(main_argv[i]) + 1;
  }
  i32_store(pargv_buf_size, buf_size);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_args_getZ_iii, (u32 argv, u32 argv_buf), {
  u32 buf_size = 0;
  for (u32 i = 0; i < main_argc; i++) {
    u32 ptr = argv_buf + buf_size;
    i32_store(argv + i * 4, ptr);
    char* arg = main_argv[i];
    strcpy(MEMACCESS(ptr), arg);
    buf_size += strlen(arg) + 1;
  }
  return 0;
});

// Maintain a stack of setjmps, each jump taking us back to the last invoke.

#define MAX_SETJMP_STACK 1024

static jmp_buf setjmp_stack[MAX_SETJMP_STACK];

static u32 next_setjmp = 0;

// Declare exports for invokes. We should generate them based on what the
// wasm needs, but for now have a fixed list here. To get things to link,
// declare them, so they either link with the existing value in the main
// wasm2c .c output file, or else they contain NULL but will never be called.

#define DECLARE_EXPORT(ret, name, args) \
__attribute__((weak)) \
ret (*WASM_RT_ADD_PREFIX(name)) args = NULL;

DECLARE_EXPORT(void, Z_setThrewZ_vii, (u32, u32));

#define VOID_INVOKE_IMPL(name, typed_args, types, args, dyncall) \
DECLARE_EXPORT(void, dyncall, types); \
\
IMPORT_IMPL(void, name, typed_args, { \
  VERBOSE_LOG("invoke " #name "  " #dyncall "\n"); \
  u32 sp = Z_stackSaveZ_iv(); \
  if (next_setjmp >= MAX_SETJMP_STACK) { \
    abort_with_message("too many nested setjmps"); \
  } \
  u32 id = next_setjmp++; \
  int result = setjmp(setjmp_stack[id]); \
  if (result == 0) { \
    (* dyncall) args; \
    /* if we got here, no longjmp or exception happened, we returned normally */ \
  } else { \
    /* A longjmp or an exception took us here. */ \
    Z_stackRestoreZ_vi(sp); \
    Z_setThrewZ_vii(1, 0); \
  } \
  next_setjmp--; \
});

#define RETURNING_INVOKE_IMPL(ret, name, typed_args, types, args, dyncall) \
DECLARE_EXPORT(ret, dyncall, types); \
\
IMPORT_IMPL(ret, name, typed_args, { \
  VERBOSE_LOG("invoke " #name "  " #dyncall "\n"); \
  u32 sp = Z_stackSaveZ_iv(); \
  if (next_setjmp >= MAX_SETJMP_STACK) { \
    abort_with_message("too many nested setjmps"); \
  } \
  u32 id = next_setjmp++; \
  int result = setjmp(setjmp_stack[id]); \
  ret returned_value = 0; \
  if (result == 0) { \
    returned_value = (* dyncall) args; \
    /* if we got here, no longjmp or exception happened, we returned normally */ \
  } else { \
    /* A longjmp or an exception took us here. */ \
    Z_stackRestoreZ_vi(sp); \
    Z_setThrewZ_vii(1, 0); \
  } \
  next_setjmp--; \
  return returned_value; \
});

VOID_INVOKE_IMPL(Z_envZ_invoke_vZ_vi, (u32 fptr), (u32), (fptr), Z_dynCall_vZ_vi);
VOID_INVOKE_IMPL(Z_envZ_invoke_viiZ_viii, (u32 fptr, u32 a, u32 b), (u32, u32, u32), (fptr, a, b), Z_dynCall_viiZ_viii);
VOID_INVOKE_IMPL(Z_envZ_invoke_viiiZ_viiii, (u32 fptr, u32 a, u32 b, u32 c), (u32, u32, u32, u32), (fptr, a, b, c), Z_dynCall_viiiZ_viiii);

RETURNING_INVOKE_IMPL(u32, Z_envZ_invoke_iiiZ_iiii, (u32 fptr, u32 a, u32 b), (u32, u32, u32), (fptr, a, b), Z_dynCall_iiiZ_iiii);
RETURNING_INVOKE_IMPL(u32, Z_envZ_invoke_iiZ_iii, (u32 fptr, u32 a), (u32, u32), (fptr, a), Z_dynCall_iiZ_iii);

IMPORT_IMPL(void, Z_envZ_emscripten_longjmpZ_vii, (u32 buf, u32 value), {
  if (next_setjmp == 0) {
    abort_with_message("longjmp without setjmp");
  }
  Z_setThrewZ_vii(buf, value ? value : 1);
  longjmp(setjmp_stack[next_setjmp - 1], 1);
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji, (u32 clock_id, u64 max_lag, u32 out), {
  // TODO: handle realtime vs monotonic etc.
  // wasi expects a result in nanoseconds, and we know how to convert clock()
  // to seconds, so compute from there
  const double NSEC_PER_SEC = 1000.0 * 1000.0 * 1000.0;
  i64_store(out, (u64)(clock() / (CLOCKS_PER_SEC / NSEC_PER_SEC)));
  return 0;
});

IMPORT_IMPL(void, Z_envZ_emscripten_notify_memory_growthZ_vi, (u32 size), {});

STUB_IMPORT_IMPL(u32, Z_envZ_pthread_createZ_iiiii, (u32 a, u32 b, u32 c, u32 d), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_joinZ_iii, (u32 a, u32 b), -1);

STUB_IMPORT_IMPL(u32, Z_envZ___cxa_thread_atexitZ_iiii, (u32 a, u32 b, u32 c), -1);

static u32 tempRet0 = 0;

IMPORT_IMPL(u32, Z_envZ_getTempRet0Z_iv, (), {
  return tempRet0;
});

IMPORT_IMPL(void, Z_envZ_setTempRet0Z_vi, (u32 x), {
  tempRet0 = x;
});

// autodebug

IMPORT_IMPL(void, Z_envZ_log_executionZ_vi, (u32 loc), {
  printf("log_execution %d\n", loc);
});
IMPORT_IMPL(u32, Z_envZ_get_i32Z_iiii, (u32 loc, u32 index, u32 value), {
  printf("get_i32 %d,%d,%d\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_get_i64Z_iiiii, (u32 loc, u32 index, u32 low, u32 high), {
  printf("get_i64 %d,%d,%d,%d\n", loc, index, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(f32, Z_envZ_get_f32Z_fiif, (u32 loc, u32 index, f32 value), {
  printf("get_f32 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_get_f64Z_diid, (u32 loc, u32 index, f64 value), {
  printf("get_f64 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_set_i32Z_iiii, (u32 loc, u32 index, u32 value), {
  printf("set_i32 %d,%d,%d\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_set_i64Z_iiiii, (u32 loc, u32 index, u32 low, u32 high), {
  printf("set_i64 %d,%d,%d,%d\n", loc, index, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(f32, Z_envZ_set_f32Z_fiif, (u32 loc, u32 index, f32 value), {
  printf("set_f32 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_set_f64Z_diid, (u32 loc, u32 index, f64 value), {
  printf("set_f64 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_load_ptrZ_iiiii, (u32 loc, u32 bytes, u32 offset, u32 ptr), {
  printf("load_ptr %d,%d,%d,%d\n", loc, bytes, offset, ptr);
  return ptr;
});
IMPORT_IMPL(u32, Z_envZ_load_val_i32Z_iii, (u32 loc, u32 value), {
  printf("load_val_i32 %d,%d\n", loc, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_load_val_i64Z_iiii, (u32 loc, u32 low, u32 high), {
  printf("load_val_i64 %d,%d,%d\n", loc, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(f32, Z_envZ_load_val_f32Z_fif, (u32 loc, f32 value), {
  printf("load_val_f32 %d,%f\n", loc, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_load_val_f64Z_did, (u32 loc, f64 value), {
  printf("load_val_f64 %d,%f\n", loc, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_store_ptrZ_iiiii, (u32 loc, u32 bytes, u32 offset, u32 ptr), {
  printf("store_ptr %d,%d,%d,%d\n", loc, bytes, offset, ptr);
  return ptr;
});
IMPORT_IMPL(u32, Z_envZ_store_val_i32Z_iii, (u32 loc, u32 value), {
  printf("store_val_i32 %d,%d\n", loc, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_store_val_i64Z_iiii, (u32 loc, u32 low, u32 high), {
  printf("store_val_i64 %d,%d,%d\n", loc, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(f32, Z_envZ_store_val_f32Z_fif, (u32 loc, f32 value), {
  printf("store_val_f32 %d,%f\n", loc, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_store_val_f64Z_did, (u32 loc, f64 value), {
  printf("store_val_f64 %d,%f\n", loc, value);
  return value;
});

// Main

int main(int argc, char** argv) {
  main_argc = argc;
  main_argv = argv;

  init_fds();

  init();

  int trap_code;
  if ((trap_code = setjmp(g_jmp_buf))) {
    printf("[wasm trap %d, halting]\n", trap_code);
    return 1;
  } else {
    Z__startZ_vv();
  }
  return 0;
}
