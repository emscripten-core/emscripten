#ifndef PTHREADFS_H
#define PTHREADFS_H

#include <wasi/api.h>
#include <thread>

#define EM_PTHREADFS_ASM(code) g_synctoasync_helper.doWork([](SyncToAsync::Callback resume) { \
    g_resumeFct = [resume]() { resume(); }; \
    EM_ASM({(async () => {code wasmTable.get($0)(); \
    })();}, &resumeWrapper_v); \
  });

#define WASI_JSAPI_DEF(name, ...) extern void __fd_##name##_async(__wasi_fd_t fd, __VA_ARGS__, void (*fun)(__wasi_errno_t));
#define WASI_JSAPI_NOARGS_DEF(name) extern void __fd_##name##_async(__wasi_fd_t fd, void (*fun)(__wasi_errno_t));

#define WASI_CAPI_DEF(name, ...) __wasi_errno_t __wasi_fd_##name(__wasi_fd_t fd, __VA_ARGS__)
#define WASI_CAPI_NOARGS_DEF(name) __wasi_errno_t __wasi_fd_##name(__wasi_fd_t fd)

#define WASI_JSAPI(name, ...) __fd_##name##_async(__VA_ARGS__)
#define WASI_SYNCTOASYNC(name, ...) g_synctoasync_helper.doWork([fd, __VA_ARGS__](SyncToAsync::Callback resume) { \
    g_resumeFct = [resume]() { resume(); }; \
    WASI_JSAPI(name, fd, __VA_ARGS__, &resumeWrapper_wasi); \
  }); \
  return resume_result_wasi;
#define WASI_SYNCTOASYNC_NOARGS(name) g_synctoasync_helper.doWork([fd](SyncToAsync::Callback resume) { \
    g_resumeFct = [resume]() { resume(); }; \
    WASI_JSAPI(name, fd, &resumeWrapper_wasi); \
  }); \
  return resume_result_wasi;

// Classic Syscalls

#define SYS_JSAPI_DEF(name, ...) extern void __sys_##name##_async(__VA_ARGS__, void (*fun)(long));
#define SYS_JSAPI_NOARGS_DEF(name) extern void __sys_##name##_async(void (*fun)(long));

#define SYS_CAPI_DEF(name, number, ...) long __syscall##number(__VA_ARGS__)

#define SYS_DEF(name, number, ...) SYS_CAPI_DEF(name, number, __VA_ARGS__); SYS_JSAPI_DEF(name, __VA_ARGS__)

#define SYS_JSAPI(name, ...) __sys_##name##_async(__VA_ARGS__)
#define SYS_SYNCTOASYNC(name, ...) g_synctoasync_helper.doWork([__VA_ARGS__](SyncToAsync::Callback resume) { \
    g_resumeFct = [resume]() { resume(); }; \
    SYS_JSAPI(name, __VA_ARGS__, &resumeWrapper_l); \
  }); \
  return resume_result_long;
#define SYS_SYNCTOASYNC_NOARGS(name) g_synctoasync_helper.doWork([](SyncToAsync::Callback resume) { \
    g_resumeFct = [resume]() { resume(); }; \
    SYS_JSAPI(name, &resumeWrapper_l); \
  }); \
  return resume_result_long;

extern "C" {
  // Helpers
  extern void init_pthreadfs(void (*fun)(void));
  extern void init_sfafs(void (*fun)(void));
  extern void init_fsafs(void (*fun)(void));
  void emscripten_init_pthreadfs();

  // WASI
  WASI_JSAPI_DEF(write, const __wasi_ciovec_t *iovs, size_t iovs_len, __wasi_size_t *nwritten)
  WASI_JSAPI_DEF(read, const __wasi_iovec_t *iovs, size_t iovs_len, __wasi_size_t *nread)
  WASI_JSAPI_DEF(pwrite, const __wasi_ciovec_t *iovs, size_t iovs_len, __wasi_filesize_t offset, __wasi_size_t *nwritten)
  WASI_JSAPI_DEF(pread, const __wasi_iovec_t *iovs, size_t iovs_len, __wasi_filesize_t offset, __wasi_size_t *nread)
  WASI_JSAPI_DEF(seek, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t *newoffset)
  WASI_JSAPI_DEF(fdstat_get, __wasi_fdstat_t *stat)
  WASI_JSAPI_NOARGS_DEF(close)
  WASI_JSAPI_NOARGS_DEF(sync)

  // Syscalls
  // see https://github.com/emscripten-core/emscripten/blob/main/system/lib/libc/musl/arch/emscripten/syscall_arch.h
SYS_CAPI_DEF(open, 5, long path, long flags, ...);
SYS_JSAPI_DEF(open, long path, long flags, int varargs)

SYS_CAPI_DEF(unlink, 10, long path);
SYS_JSAPI_DEF(unlink, long path)

SYS_CAPI_DEF(chdir, 12, long path);
SYS_JSAPI_DEF(chdir, long path)

SYS_CAPI_DEF(mknod, 14, long path, long mode, long dev);
SYS_JSAPI_DEF(mknod, long path, long mode, long dev)

SYS_CAPI_DEF(chmod, 15, long path, long mode);
SYS_JSAPI_DEF(chmod, long path, long mode)

SYS_CAPI_DEF(access, 33, long path, long amode);
SYS_JSAPI_DEF(access, long path, long amode)

SYS_CAPI_DEF(rename, 38, long old_path, long new_path);
SYS_JSAPI_DEF(rename, long old_path, long new_path)

SYS_CAPI_DEF(mkdir, 39, long path, long mode);
SYS_JSAPI_DEF(mkdir, long path, long mode)

SYS_CAPI_DEF(rmdir, 40, long path);
SYS_JSAPI_DEF(rmdir, long path)

SYS_CAPI_DEF(dup, 41, long fd);
SYS_JSAPI_DEF(dup, long fd)

SYS_CAPI_DEF(ioctl, 54, long fd, long request, ...);
SYS_JSAPI_DEF(ioctl, long fd, long request, void *const varargs)

SYS_CAPI_DEF(dup2, 63, long oldfd, long newfd);
SYS_JSAPI_DEF(dup2, long oldfd, long newfd)

SYS_CAPI_DEF(symlink, 83, long target, long linkpath);
SYS_JSAPI_DEF(symlink, long target, long linkpath)

SYS_CAPI_DEF(readlink, 85, long path, long buf, long bufsize);
SYS_JSAPI_DEF(readlink, long path, long buf, long bufsize)

SYS_CAPI_DEF(munmap, 91, long addr, long len);
SYS_JSAPI_DEF(munmap, long addr, long len)

SYS_CAPI_DEF(fchmod, 94, long fd, long mode);
SYS_JSAPI_DEF(fchmod, long fd, long mode)

SYS_CAPI_DEF(fchdir, 133, long fd);
SYS_JSAPI_DEF(fchdir, long fd)

SYS_CAPI_DEF(_newselect, 142, long nfds, long readfds, long writefds, long exceptfds, long timeout);
SYS_JSAPI_DEF(_newselect, long nfds, long readfds, long writefds, long exceptfds, long timeout)

SYS_CAPI_DEF(msync, 144, long addr, long len, long flags);
SYS_JSAPI_DEF(msync, long addr, long len, long flags)

SYS_CAPI_DEF(fdatasync, 148, long fd);
SYS_JSAPI_DEF(fdatasync, long fd)

SYS_CAPI_DEF(poll, 168, long fds, long nfds, long timeout);
SYS_JSAPI_DEF(poll, long fds, long nfds, long timeout)

SYS_CAPI_DEF(getcwd, 183, long buf, long size);
SYS_JSAPI_DEF(getcwd, long buf, long size)

SYS_CAPI_DEF(mmap2, 192, long addr, long len, long prot, long flags, long fd, long off);
SYS_JSAPI_DEF(mmap2, long addr, long len, long prot, long flags, long fd, long off)

SYS_CAPI_DEF(truncate64, 193, long path, long zero, long low, long high);
SYS_JSAPI_DEF(truncate64, long path, long zero, long low, long high)

SYS_CAPI_DEF(ftruncate64, 194, long fd, long zero, long low, long high);
SYS_JSAPI_DEF(ftruncate64, long fd, long zero, long low, long high)

SYS_CAPI_DEF(stat64, 195, long path, long buf);
SYS_JSAPI_DEF(stat64, long path, long buf)

SYS_CAPI_DEF(lstat64, 196, long path, long buf);
SYS_JSAPI_DEF(lstat64, long path, long buf)

SYS_CAPI_DEF(fstat64, 197, long fd, long buf);
SYS_JSAPI_DEF(fstat64, long fd, long buf)

SYS_CAPI_DEF(lchown32, 198, long path, long owner, long group);
SYS_JSAPI_DEF(lchown32, long path, long owner, long group)

SYS_CAPI_DEF(fchown32, 207, long fd, long owner, long group);
SYS_JSAPI_DEF(fchown32, long fd, long owner, long group)

SYS_CAPI_DEF(chown32, 212, long path, long owner, long group);
SYS_JSAPI_DEF(chown32, long path, long owner, long group)

SYS_CAPI_DEF(getdents64, 220, long fd, long dirp, long count);
SYS_JSAPI_DEF(getdents64, long fd, long dirp, long count)

SYS_CAPI_DEF(fcntl64, 221, long fd, long cmd, ...);
SYS_JSAPI_DEF(fcntl64, long fd, long cmd, int varargs)

SYS_CAPI_DEF(statfs64, 268, long path, long size, long buf);
SYS_JSAPI_DEF(statfs64, long path, long size, long buf)

SYS_CAPI_DEF(fstatfs64, 269, long fd, long size, long buf);
SYS_JSAPI_DEF(fstatfs64, long fd, long size, long buf)

SYS_CAPI_DEF(openat, 295, long dirfd, long path, long flags, ...);
SYS_JSAPI_DEF(openat, long dirfd, long path, long flags, int varargs)

SYS_CAPI_DEF(mkdirat, 296, long dirfd, long path, long mode);
SYS_JSAPI_DEF(mkdirat, long dirfd, long path, long mode)

SYS_CAPI_DEF(mknodat, 297, long dirfd, long path, long mode, long dev);
SYS_JSAPI_DEF(mknodat, long dirfd, long path, long mode, long dev)

SYS_CAPI_DEF(fchownat, 298, long dirfd, long path, long owner, long group, long flags);
SYS_JSAPI_DEF(fchownat, long dirfd, long path, long owner, long group, long flags)

SYS_CAPI_DEF(fstatat64, 300, long dirfd, long path, long buf, long flags);
SYS_JSAPI_DEF(fstatat64, long dirfd, long path, long buf, long flags)

SYS_CAPI_DEF(unlinkat, 301, long dirfd, long path, long flags);
SYS_JSAPI_DEF(unlinkat, long dirfd, long path, long flags)

SYS_CAPI_DEF(renameat, 302, long olddirfd, long oldpath, long newdirfd, long newpath);
SYS_JSAPI_DEF(renameat, long olddirfd, long oldpath, long newdirfd, long newpath)

SYS_CAPI_DEF(symlinkat, 304, long target, long newdirfd, long linkpath);
SYS_JSAPI_DEF(symlinkat, long target, long newdirfd, long linkpath)

SYS_CAPI_DEF(readlinkat, 305, long dirfd, long path, long bug, long bufsize);
SYS_JSAPI_DEF(readlinkat, long dirfd, long path, long bug, long bufsize)

SYS_CAPI_DEF(fchmodat, 306, long dirfd, long path, long mode, ...);
SYS_JSAPI_DEF(fchmodat, long dirfd, long path, long mode, int varargs)

SYS_CAPI_DEF(faccessat, 307, long dirfd, long path, long amode, long flags);
SYS_JSAPI_DEF(faccessat, long dirfd, long path, long amode, long flags)

SYS_CAPI_DEF(utimensat, 320, long dirfd, long path, long times, long flags);
SYS_JSAPI_DEF(utimensat, long dirfd, long path, long times, long flags)

SYS_CAPI_DEF(fallocate, 324, long fd, long mode, long off_low, long off_high, long len_low, long len_high);
SYS_JSAPI_DEF(fallocate, long fd, long mode, long off_low, long off_high, long len_low, long len_high)

SYS_CAPI_DEF(dup3, 330, long fd, long suggestfd, long flags);
SYS_JSAPI_DEF(dup3, long fd, long suggestfd, long flags)
}

class SyncToAsync {
public:
  using Callback = std::function<void()>;

  SyncToAsync();

  ~SyncToAsync();

  void shutdown();

  // Run some work on thread. This is a synchronous call, but the thread can do
  // async work for us. To allow us to know when the async work finishes, the
  // worker is given a function to call at that time.
  void doWork(std::function<void(Callback)> newWork);

private:
  std::thread thread;
  std::mutex mutex;
  std::condition_variable condition;
  std::function<void(Callback)> work;
  bool readyToWork = false;
  bool finishedWork;
  bool quit = false;

  // The child will be asynchronous, and therefore we cannot rely on RAII to
  // unlock for us, we must do it manually.
  std::unique_lock<std::mutex> childLock;

  static void* threadMain(void* arg);

  static void threadIter(void* arg);
};

// Declare global variables to be populated by resume;
extern SyncToAsync::Callback g_resumeFct;
extern SyncToAsync g_synctoasync_helper;

// Static functions calling resumFct and setting corresponding the return value.
void resumeWrapper_v();

void resumeWrapper_l(long retVal);

void resumeWrapper_wasi(__wasi_errno_t retVal);

void emscripten_init_pthreadfs();

#endif  // PTHREADFS_H