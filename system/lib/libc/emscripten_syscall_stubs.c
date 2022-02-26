/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Unimplemented/dummy syscall implemenations. These fall into 3 catagories.
 *
 * 1. Fake it, use dummy/placeholder values and return success.
 * 2. Fake it, as above but warn at runtime if called.
 * 3. Return ENOSYS and warn at runtime if called.
 */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <syscall_arch.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/utsname.h>
#include <emscripten/console.h>

static int g_pid = 42;
static int g_pgid = 42;
static int g_ppid = 1;
static int g_sid = 42;
static mode_t g_umask = S_IRWXU | S_IRWXG | S_IRWXO;

#ifdef NDEBUG
#define REPORT(name)
#else
#define REPORT(name) \
  emscripten_console_error("warning: unsupported syscall: __syscall_" #name "\n");
#endif

#define UNIMPLEMENTED(name, args) \
  long __syscall_##name args { \
    REPORT(name); \
    return -ENOSYS; \
  }

long __syscall_uname(long buf) {
  if (!buf) {
    return -EFAULT;
  }
  struct utsname *utsname = (struct utsname *)buf;

  strcpy(utsname->sysname, "Emscripten");
  strcpy(utsname->nodename, "emscripten");
  strcpy(utsname->release, "1.0");
  strcpy(utsname->version, "#1");
#ifdef __wams64__
  strcpy(utsname->machine, "wasm64");
#else
  strcpy(utsname->machine, "wasm32");
#endif
  return 0;
}

long __syscall_setpgid(long pid, long pgid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  if (pgid && pgid != g_pgid) {
    return -EPERM;
  }
  return 0;
}

long __syscall_sync() {
  return 0;
}

long __syscall_getsid(long pid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  return g_sid;
}

long __syscall_getpgid(long pid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  return g_pgid;
}

long __syscall_getpid() {
  return g_pid;
}

long __syscall_getppid() {
  return g_ppid;
}

long __syscall_link(long oldpath, long newpath) {
  return -EMLINK; // no hardlinks for us
}

long __syscall_nice(long inc) {
  return -EPERM; // no meaning to nice for our single-process environment
}

long __syscall_getgroups32(long size, long list) {
  if (size < 1) {
    return -EINVAL;
  }
  ((gid_t*)list)[0] = 0;
  return 1;
}

long __syscall_setsid() {
  return 0; // no-op
}

long  __syscall_umask(long mask) {
  long old = g_umask;
  g_umask = mask;
  return old;
}

long __syscall_setrlimit(long resource, long limit) {
  return 0; // no-op
}

long __syscall_getrusage(long who, long usage) {
  REPORT(getrusage);
  struct rusage *u = (struct rusage *)usage;
  memset(u, 0, sizeof(*u));
  u->ru_utime.tv_sec = 1;
  u->ru_utime.tv_usec = 2;
  u->ru_stime.tv_sec = 3;
  u->ru_stime.tv_usec = 4;
  return 0;
}

long __syscall_getpriority(long which, long who) {
  return 0;
}

long __syscall_setpriority(long which, long who, long prio) {
  return -EPERM;
}

long __syscall_setdomainname(long name, long size) {
  return -EPERM;
}

long __syscall_getresuid32(long ruid, long euid, long suid) {
  *((uid_t *)ruid) = 0;
  *((uid_t *)euid) = 0;
  *((uid_t *)suid) = 0;
  return 0;
}

long __syscall_getresgid32(long ruid, long euid, long suid) {
  REPORT(getresgid32);
  *((uid_t *)ruid) = 0;
  *((uid_t *)euid) = 0;
  *((uid_t *)suid) = 0;
  return 0;
}

long __syscall_pause() {
  REPORT(pause);
  return -EINTR; // we can't pause
}

long __syscall_madvise(long addr, long length, long advice) {
  REPORT(madvise);
  // advice is welcome, but ignored
  return 0;
}

long __syscall_mlock(long addr, long len) {
  REPORT(mlock);
  return 0;
}

long __syscall_munlock(long addr, long len) {
  REPORT(munlock);
  return 0;
}

long __syscall_mprotect(long addr, long len, long size) {
  REPORT(mprotect);
  return 0; // let's not and say we did
}

long __syscall_mremap(long old_addr, long old_size, long new_size, long flags, long new_addr) {
  REPORT(mremap);
  return -ENOMEM; // never succeed
}

long __syscall_mlockall(long flags) {
  REPORT(mlockall);
  return 0;
}

long __syscall_munlockall() {
  REPORT(munlockall);
  return 0;
}

long __syscall_prlimit64(long pid, long resource, long new_limit, long old_limit) {
  REPORT(prlimit64);
  struct rlimit *old = (struct rlimit *)old_limit;
  if (old) { // just report no limits
    old->rlim_cur = RLIM_INFINITY;
    old->rlim_max = RLIM_INFINITY;
  }
  return 0;
}

long __syscall_ugetrlimit(long resource, long rlim) {
  REPORT(ugetrlimit);
  struct rlimit * limits = (struct rlimit *)rlim;
  limits->rlim_cur = RLIM_INFINITY;
  limits->rlim_max = RLIM_INFINITY;
  return 0; // just report no limits
}

long __syscall_setsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  REPORT(setsockopt);
  return -ENOPROTOOPT; // The option is unknown at the level indicated.
}

UNIMPLEMENTED(acct, (long filename))
UNIMPLEMENTED(mincore, (long addr, long length, long vec))
UNIMPLEMENTED(pipe2, (long fds, long flags))
UNIMPLEMENTED(pselect6, (long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks))
UNIMPLEMENTED(recvmmsg, (long sockfd, long msgvec, long vlen, long flags, ...))
UNIMPLEMENTED(sendmmsg, (long sockfd, long msgvec, long vlen, long flags, ...))
UNIMPLEMENTED(setitimer, (long which, long new_value, long old_value))
UNIMPLEMENTED(getitimer, (long which, long old_value))
UNIMPLEMENTED(shutdown, (long sockfd, long level, long optname, long optval, long optlen, long dummy))
UNIMPLEMENTED(socketpair, (long sockfd, long level, long optname, long optval, long optlen, long dummy))
UNIMPLEMENTED(wait4,(long pid, long wstatus, long options, long rusage))
