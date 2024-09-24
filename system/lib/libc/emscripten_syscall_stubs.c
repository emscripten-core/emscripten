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
#include <emscripten/version.h>

static int g_pid = 42;
static int g_pgid = 42;
static int g_ppid = 1;
static int g_sid = 42;
static mode_t g_umask = S_IWGRP | S_IWOTH;

#ifdef NDEBUG
#define REPORT(name)
#else
#define REPORT(name) \
  emscripten_err("warning: unsupported syscall: __syscall_" #name "\n");
#endif

#define UNIMPLEMENTED(name, args) \
  weak int __syscall_##name args { \
    REPORT(name); \
    return -ENOSYS; \
  }

#define STRINGIFY(s) #s
#define STR(s) STRINGIFY(s)

weak int __syscall_uname(intptr_t buf) {
  if (!buf) {
    return -EFAULT;
  }
  const char* full_version = STR(__EMSCRIPTEN_major__) "." \
                             STR(__EMSCRIPTEN_minor__) "." \
                             STR(__EMSCRIPTEN_tiny__);

  struct utsname *utsname = (struct utsname *)buf;

  strcpy(utsname->sysname, "Emscripten");
  strcpy(utsname->nodename, "emscripten");
  strcpy(utsname->release, full_version);
  strcpy(utsname->version, "#1");
#ifdef __wasm64__
  strcpy(utsname->machine, "wasm64");
#else
  strcpy(utsname->machine, "wasm32");
#endif
  return 0;
}

weak int __syscall_setpgid(int pid, int pgid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  if (pgid && pgid != g_pgid) {
    return -EPERM;
  }
  return 0;
}

weak int __syscall_sync() {
  return 0;
}

weak int __syscall_getsid(int pid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  return g_sid;
}

weak int __syscall_getpgid(int pid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  return g_pgid;
}

weak int __syscall_getpid() {
  return g_pid;
}

weak int __syscall_getppid() {
  return g_ppid;
}

weak int __syscall_linkat(int olddirfd, intptr_t oldpath, int newdirfd, intptr_t newpath, int flags) {
  return -EMLINK; // no hardlinks for us
}

weak int __syscall_getgroups32(int size, intptr_t list) {
  if (size < 1) {
    return -EINVAL;
  }
  ((gid_t*)list)[0] = 0;
  return 1;
}

weak int __syscall_setsid() {
  return 0; // no-op
}

weak int __syscall_umask(int mask) {
  int old = g_umask;
  g_umask = mask;
  return old;
}

weak int __syscall_setrlimit(int resource, intptr_t limit) {
  return 0; // no-op
}

weak int __syscall_getrusage(int who, intptr_t usage) {
  REPORT(getrusage);
  struct rusage *u = (struct rusage *)usage;
  memset(u, 0, sizeof(*u));
  u->ru_utime.tv_sec = 1;
  u->ru_utime.tv_usec = 2;
  u->ru_stime.tv_sec = 3;
  u->ru_stime.tv_usec = 4;
  return 0;
}

weak int __syscall_getpriority(int which, int who) {
  return 0;
}

weak int __syscall_setpriority(int which, int who, int prio) {
  return -EPERM;
}

weak int __syscall_setdomainname(intptr_t name, size_t size) {
  return -EPERM;
}

weak int __syscall_getuid32(void) {
  return 0;
}

weak int __syscall_getgid32(void) {
  return 0;
}

weak int __syscall_geteuid32(void) {
  return 0;
}

weak int __syscall_getegid32(void) {
  return 0;
}

weak int __syscall_getresuid32(intptr_t ruid, intptr_t euid, intptr_t suid) {
  *((uid_t *)ruid) = 0;
  *((uid_t *)euid) = 0;
  *((uid_t *)suid) = 0;
  return 0;
}

weak int __syscall_getresgid32(intptr_t ruid, intptr_t euid, intptr_t suid) {
  REPORT(getresgid32);
  *((uid_t *)ruid) = 0;
  *((uid_t *)euid) = 0;
  *((uid_t *)suid) = 0;
  return 0;
}

weak int __syscall_pause() {
  REPORT(pause);
  return -EINTR; // we can't pause
}

weak int __syscall_madvise(intptr_t addr, size_t length, int advice) {
  REPORT(madvise);
  // advice is welcome, but ignored
  return 0;
}

weak int __syscall_mlock(intptr_t addr, size_t len) {
  REPORT(mlock);
  return 0;
}

weak int __syscall_munlock(intptr_t addr, size_t len) {
  REPORT(munlock);
  return 0;
}

weak int __syscall_mprotect(size_t addr, size_t len, int prot) {
  REPORT(mprotect);
  return 0; // let's not and say we did
}

weak int __syscall_mremap(intptr_t old_addr, size_t old_size, size_t new_size, int flags, intptr_t new_addr) {
  REPORT(mremap);
  return -ENOMEM; // never succeed
}

weak int __syscall_mlockall(int flags) {
  REPORT(mlockall);
  return 0;
}

weak int __syscall_munlockall() {
  REPORT(munlockall);
  return 0;
}

weak int __syscall_prlimit64(int pid, int resource, intptr_t new_limit, intptr_t old_limit) {
  REPORT(prlimit64);
  struct rlimit *old = (struct rlimit *)old_limit;
  if (old) { // just report no limits
    old->rlim_cur = RLIM_INFINITY;
    old->rlim_max = RLIM_INFINITY;
  }
  return 0;
}

weak int __syscall_ugetrlimit(int resource, intptr_t rlim) {
  REPORT(ugetrlimit);
  struct rlimit * limits = (struct rlimit *)rlim;
  limits->rlim_cur = RLIM_INFINITY;
  limits->rlim_max = RLIM_INFINITY;
  return 0; // just report no limits
}

weak int __syscall_setsockopt(int sockfd, int level, int optname, intptr_t optval, size_t optlen, int dummy) {
  REPORT(setsockopt);
  return -ENOPROTOOPT; // The option is unknown at the level indicated.
}

UNIMPLEMENTED(acct, (intptr_t filename))
UNIMPLEMENTED(mincore, (intptr_t addr, size_t length, intptr_t vec))
UNIMPLEMENTED(pipe2, (intptr_t fds, int flags))
UNIMPLEMENTED(pselect6, (int nfds, intptr_t readfds, intptr_t writefds, intptr_t exceptfds, intptr_t timeout, intptr_t sigmaks))
UNIMPLEMENTED(recvmmsg, (int sockfd, intptr_t msgvec, size_t vlen, int flags, ...))
UNIMPLEMENTED(sendmmsg, (int sockfd, intptr_t msgvec, size_t vlen, int flags, ...))
UNIMPLEMENTED(shutdown, (int sockfd, int how, int dummy, int dummy2, int dummy3, int dummy4))
UNIMPLEMENTED(socketpair, (int domain, int type, int protocol, intptr_t fds, int dummy, int dummy2))
UNIMPLEMENTED(wait4,(int pid, intptr_t wstatus, int options, int rusage))
