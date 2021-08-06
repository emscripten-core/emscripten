#include "pthreadfs.h"

#include <assert.h>
#include <emscripten.h>
#include <pthread.h>
#include <wasi/api.h>

#include <thread>
#include <functional>
#include <iostream>

#include <stdarg.h>

SyncToAsync::SyncToAsync() : thread(threadMain, this), childLock(mutex) {
  // The child lock is associated with the mutex, which takes the lock, and
  // we free it here. Only the child will lock/unlock it from now on.
  childLock.unlock();
}

SyncToAsync::~SyncToAsync() {
  quit = true;

  shutdown();

  thread.join();
}

void SyncToAsync::shutdown() {
  readyToWork = true;
  condition.notify_one();
}

void SyncToAsync::doWork(std::function<void(SyncToAsync::Callback)> newWork) {
  // Send the work over.
  {
    std::lock_guard<std::mutex> lock(mutex);
    work = newWork;
    finishedWork = false;
    readyToWork = true;
  }
  condition.notify_one();

  // Wait for it to be complete.
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock, [&]() {
    return finishedWork;
  });
}

void* SyncToAsync::threadMain(void* arg) {
  // Prevent the pthread from shutting down too early.
  EM_ASM(runtimeKeepalivePush(););
  auto* parent = (SyncToAsync*)arg;
  emscripten_async_call(threadIter, arg, 0);
  return 0;
}

void SyncToAsync::threadIter(void* arg) {
  auto* parent = (SyncToAsync*)arg;
  // Wait until we get something to do.
  parent->childLock.lock();
  parent->condition.wait(parent->childLock, [&]() {
    return parent->readyToWork;
  });
  if (parent->quit) {
    EM_ASM(runtimeKeepalivePop(););
    return;
  }
  auto work = parent->work;
  parent->readyToWork = false;
  // Do the work.
  work([parent, arg]() {
    // We are called, so the work was finished. Notify the caller.
    parent->finishedWork = true;
    parent->childLock.unlock();
    parent->condition.notify_one();
    threadIter(arg);
  });
}

// Define global variables to be populated by resume;
SyncToAsync::Callback g_resumeFct;
SyncToAsync g_synctoasync_helper;

// Static functions calling resumFct and setting the return value.
void resumeWrapper_v()
{
  g_resumeFct();
}
// return value long
long resume_result_long = 0;
void resumeWrapper_l(long retVal)
{
  resume_result_long = retVal;
  g_resumeFct();
}
// return value __wasi_errno_t
__wasi_errno_t resume_result_wasi = 0;
void resumeWrapper_wasi(__wasi_errno_t retVal)
{
  resume_result_wasi = retVal;
  g_resumeFct();
}

// Wasi definitions
WASI_CAPI_DEF(write, const __wasi_ciovec_t *iovs, size_t iovs_len, __wasi_size_t *nwritten) {
  WASI_SYNCTOASYNC(write, iovs, iovs_len, nwritten);
}
WASI_CAPI_DEF(read, const __wasi_iovec_t *iovs, size_t iovs_len, __wasi_size_t *nread) {
  WASI_SYNCTOASYNC(read, iovs, iovs_len, nread);
}
WASI_CAPI_DEF(pwrite, const __wasi_ciovec_t *iovs, size_t iovs_len, __wasi_filesize_t offset, __wasi_size_t *nwritten) {
  WASI_SYNCTOASYNC(pwrite, iovs, iovs_len, offset, nwritten);
}
WASI_CAPI_DEF(pread, const __wasi_iovec_t *iovs, size_t iovs_len, __wasi_filesize_t offset, __wasi_size_t *nread) {
  WASI_SYNCTOASYNC(pread, iovs, iovs_len, offset, nread);
}
WASI_CAPI_DEF(seek, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t *newoffset) {
  WASI_SYNCTOASYNC(seek, offset, whence, newoffset);
}
WASI_CAPI_DEF(fdstat_get, __wasi_fdstat_t *stat) {
  WASI_SYNCTOASYNC(fdstat_get, stat);
}
WASI_CAPI_NOARGS_DEF(close) {
  WASI_SYNCTOASYNC_NOARGS(close);
}
WASI_CAPI_NOARGS_DEF(sync) {
  WASI_SYNCTOASYNC_NOARGS(sync);
}

// Syscall definitions
SYS_CAPI_DEF(open, 5, long path, long flags, ...) {
  va_list vl;
  va_start(vl, flags);
  int varargs = va_arg(vl, int);
  va_end(vl);
  
  SYS_SYNCTOASYNC(open, path, flags, varargs);
}

SYS_CAPI_DEF(unlink, 10, long path) {
  SYS_SYNCTOASYNC(unlink, path);
}

SYS_CAPI_DEF(chdir, 12, long path) {
  SYS_SYNCTOASYNC(chdir, path);
}

SYS_CAPI_DEF(mknod, 14, long path, long mode, long dev) {
  SYS_SYNCTOASYNC(mknod, path, mode, dev);
}

SYS_CAPI_DEF(chmod, 15, long path, long mode) {
  SYS_SYNCTOASYNC(chmod, path, mode);
}

SYS_CAPI_DEF(access, 33, long path, long amode) {
  SYS_SYNCTOASYNC(access, path, amode);
}

SYS_CAPI_DEF(rename, 38, long old_path, long new_path) {
  SYS_SYNCTOASYNC(rename, old_path, new_path);
}

SYS_CAPI_DEF(mkdir, 39, long path, long mode) {
  SYS_SYNCTOASYNC(mkdir, path, mode);
}

SYS_CAPI_DEF(rmdir, 40, long path) {
  SYS_SYNCTOASYNC(rmdir, path);
}

SYS_CAPI_DEF(dup, 41, long fd) {
  SYS_SYNCTOASYNC(dup, fd);
}

SYS_CAPI_DEF(ioctl, 54, long fd, long request, ...) {
  void *arg;
	va_list ap;
	va_start(ap, request);
	arg = va_arg(ap, void *);
	va_end(ap);
  
  SYS_SYNCTOASYNC(ioctl, fd, request, arg);
}

SYS_CAPI_DEF(dup2, 63, long oldfd, long newfd) {
  SYS_SYNCTOASYNC(dup2, oldfd, newfd);
}

SYS_CAPI_DEF(symlink, 83, long target, long linkpath) {
  SYS_SYNCTOASYNC(symlink, target, linkpath);
}

SYS_CAPI_DEF(readlink, 85, long path, long buf, long bufsize) {
  SYS_SYNCTOASYNC(readlink, path, buf, bufsize);
}

SYS_CAPI_DEF(munmap, 91, long addr, long len) {
  SYS_SYNCTOASYNC(munmap, addr, len);
}

SYS_CAPI_DEF(fchmod, 94, long fd, long mode) {
  SYS_SYNCTOASYNC(fchmod, fd, mode);
}

SYS_CAPI_DEF(fchdir, 133, long fd) {
  SYS_SYNCTOASYNC(fchdir, fd);
}

SYS_CAPI_DEF(_newselect, 142, long nfds, long readfds, long writefds, long exceptfds, long timeout) {
  SYS_SYNCTOASYNC(_newselect, nfds, readfds, writefds, exceptfds, timeout);
}

SYS_CAPI_DEF(msync, 144, long addr, long len, long flags) {
  SYS_SYNCTOASYNC(msync, addr, len, flags);
}

SYS_CAPI_DEF(fdatasync, 148, long fd) {
  SYS_SYNCTOASYNC(fdatasync, fd);
}

SYS_CAPI_DEF(poll, 168, long fds, long nfds, long timeout) {
  SYS_SYNCTOASYNC(poll, fds, nfds, timeout);
}

SYS_CAPI_DEF(getcwd, 183, long buf, long size) {
  SYS_SYNCTOASYNC(getcwd, buf, size);
}

SYS_CAPI_DEF(mmap2, 192, long addr, long len, long prot, long flags, long fd, long off) {
  SYS_SYNCTOASYNC(mmap2, addr, len, prot, flags, fd, off);
}

SYS_CAPI_DEF(truncate64, 193, long path, long zero, long low, long high) {
  SYS_SYNCTOASYNC(truncate64, path, zero, low, high);
}

SYS_CAPI_DEF(ftruncate64, 194, long fd, long zero, long low, long high) {
  SYS_SYNCTOASYNC(ftruncate64, fd, zero, low, high);
}

SYS_CAPI_DEF(stat64, 195, long path, long buf) {
  SYS_SYNCTOASYNC(stat64, path, buf);
}

SYS_CAPI_DEF(lstat64, 196, long path, long buf) {
  SYS_SYNCTOASYNC(lstat64, path, buf);
}

SYS_CAPI_DEF(fstat64, 197, long fd, long buf) {
  SYS_SYNCTOASYNC(fstat64, fd, buf);
}

SYS_CAPI_DEF(lchown32, 198, long path, long owner, long group) {
  SYS_SYNCTOASYNC(lchown32, path, owner, group);
}

SYS_CAPI_DEF(fchown32, 207, long fd, long owner, long group) {
  SYS_SYNCTOASYNC(fchown32, fd, owner, group);
}

SYS_CAPI_DEF(chown32, 212, long path, long owner, long group) {
  SYS_SYNCTOASYNC(chown32, path, owner, group);
}

SYS_CAPI_DEF(getdents64, 220, long fd, long dirp, long count) {
  SYS_SYNCTOASYNC(getdents64, fd, dirp, count);
}

SYS_CAPI_DEF(fcntl64, 221, long fd, long cmd, ...) {
va_list vl;
  va_start(vl, cmd);
  int varargs = va_arg(vl, int);
  va_end(vl);
  
  SYS_SYNCTOASYNC(fcntl64, fd, cmd, varargs);
}

SYS_CAPI_DEF(statfs64, 268, long path, long size, long buf) {
  SYS_SYNCTOASYNC(statfs64, path, size, buf);
}

SYS_CAPI_DEF(fstatfs64, 269, long fd, long size, long buf) {
  SYS_SYNCTOASYNC(fstatfs64, fd, size, buf);
}

SYS_CAPI_DEF(openat, 295, long dirfd, long path, long flags, ...) {
va_list vl;
  va_start(vl, flags);
  int varargs = va_arg(vl, int);
  va_end(vl);
  
  SYS_SYNCTOASYNC(openat, dirfd, path, flags, varargs);
}

SYS_CAPI_DEF(mkdirat, 296, long dirfd, long path, long mode) {
  SYS_SYNCTOASYNC(mkdirat, dirfd, path, mode);
}

SYS_CAPI_DEF(mknodat, 297, long dirfd, long path, long mode, long dev) {
  SYS_SYNCTOASYNC(mknodat, dirfd, path, mode, dev);
}

SYS_CAPI_DEF(fchownat, 298, long dirfd, long path, long owner, long group, long flags) {
  SYS_SYNCTOASYNC(fchownat, dirfd, path, owner, group, flags);
}

SYS_CAPI_DEF(fstatat64, 300, long dirfd, long path, long buf, long flags) {
  SYS_SYNCTOASYNC(fstatat64, dirfd, path, buf, flags);
}

SYS_CAPI_DEF(unlinkat, 301, long dirfd, long path, long flags) {
  SYS_SYNCTOASYNC(unlinkat, dirfd, path, flags);
}

SYS_CAPI_DEF(renameat, 302, long olddirfd, long oldpath, long newdirfd, long newpath) {
  SYS_SYNCTOASYNC(renameat, olddirfd, oldpath, newdirfd, newpath);
}

SYS_CAPI_DEF(symlinkat, 304, long target, long newdirfd, long linkpath) {
  SYS_SYNCTOASYNC(symlinkat, target, newdirfd, linkpath);
}

SYS_CAPI_DEF(readlinkat, 305, long dirfd, long path, long bug, long bufsize) {
  SYS_SYNCTOASYNC(readlinkat, dirfd, path, bug, bufsize);
}

SYS_CAPI_DEF(fchmodat, 306, long dirfd, long path, long mode, ...) {
va_list vl;
  va_start(vl, mode);
  int varargs = va_arg(vl, int);
  va_end(vl);
  
  SYS_SYNCTOASYNC(fchmodat, dirfd, path, mode, varargs);
}

SYS_CAPI_DEF(faccessat, 307, long dirfd, long path, long amode, long flags) {
  SYS_SYNCTOASYNC(faccessat, dirfd, path, amode, flags);
}

SYS_CAPI_DEF(utimensat, 320, long dirfd, long path, long times, long flags) {
  SYS_SYNCTOASYNC(utimensat, dirfd, path, times, flags);
}

SYS_CAPI_DEF(fallocate, 324, long fd, long mode, long off_low, long off_high, long len_low, long len_high) {
  SYS_SYNCTOASYNC(fallocate, fd, mode, off_low, off_high, len_low, len_high);
}

SYS_CAPI_DEF(dup3, 330, long fd, long suggestfd, long flags) {
  SYS_SYNCTOASYNC(dup3, fd, suggestfd, flags);
}

// Other helper code

void emscripten_init_pthreadfs() {
  g_synctoasync_helper.doWork([](SyncToAsync::Callback resume) {
    g_resumeFct = [resume]() { resume(); };
    init_pthreadfs(&resumeWrapper_v);
  });
  g_synctoasync_helper.doWork([](SyncToAsync::Callback resume) {
    g_resumeFct = [resume]() { resume(); };
    init_fsafs(&resumeWrapper_v);
  });
  return;
}