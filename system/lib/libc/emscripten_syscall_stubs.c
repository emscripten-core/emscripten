/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Unimplemented/dummy syscall implementations. These fall into 3 catagories.
 *
 * 1. Fake it, use dummy/placeholder values and return success.
 * 2. Fake it, as above but warn at runtime if called.
 * 3. Return ENOSYS and warn at runtime if called.
 */

#define _GNU_SOURCE // for struct mmsghdr
#include <errno.h>
#include <string.h>
#include <syscall_arch.h>
#include <emscripten/console.h>
#include <emscripten/version.h>
#include <emscripten/stack.h>

static pid_t g_pid = 42;
static pid_t g_pgid = 42;
static pid_t g_ppid = 1;
static pid_t g_sid = 42;
static mode_t g_umask = S_IWGRP | S_IWOTH;

#ifdef NDEBUG
#define REPORT(name)
#else
#define REPORT(name) \
  emscripten_err("warning: unsupported syscall: __syscall_" #name);
#endif

#define UNIMPLEMENTED(name, args) \
  weak int __syscall_##name args { \
    REPORT(name); \
    return -ENOSYS; \
  }

#define STRINGIFY(s) #s
#define STR(s) STRINGIFY(s)

weak int __syscall_uname(struct utsname *buf) {
  if (!buf) {
    return -EFAULT;
  }
  const char* full_version = STR(__EMSCRIPTEN_major__) "." \
                             STR(__EMSCRIPTEN_minor__) "." \
                             STR(__EMSCRIPTEN_tiny__);

  strcpy(buf->sysname, "Emscripten");
  strcpy(buf->nodename, "emscripten");
  strcpy(buf->release, full_version);
  strcpy(buf->version, "#1");
#ifdef __wasm64__
  strcpy(buf->machine, "wasm64");
#else
  strcpy(buf->machine, "wasm32");
#endif
  return 0;
}

weak int __syscall_setpgid(pid_t pid, pid_t pgid) {
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

weak pid_t __syscall_getsid(pid_t pid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  return g_sid;
}

weak pid_t __syscall_getpgid(pid_t pid) {
  if (pid && pid != g_pid) {
    return -ESRCH;
  }
  return g_pgid;
}

weak pid_t __syscall_getpid() {
  return g_pid;
}

weak pid_t __syscall_getppid() {
  return g_ppid;
}

weak int __syscall_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags) {
  return -EMLINK; // no hardlinks for us
}

weak int __syscall_getgroups32(int count, gid_t list[]) {
  if (count < 1) {
    return -EINVAL;
  }
  list[0] = 0;
  return 1;
}

weak pid_t __syscall_setsid() {
  return 0; // no-op
}

weak mode_t __syscall_umask(mode_t mask) {
  mode_t old = g_umask;
  g_umask = mask;
  return old;
}

struct kusage {
  long utime_tv_sec;
  long utime_tv_usec;
  long stime_tv_sec;
  long stime_tv_usec;
};

weak int __syscall_getrusage(int who, void *usage) {
  REPORT(getrusage);
  struct kusage *u = (struct kusage*)usage;
  u->utime_tv_sec = 1;
  u->utime_tv_usec = 2;
  u->stime_tv_sec = 3;
  u->stime_tv_usec = 4;
  return 0;
}

weak int __syscall_getpriority(int which, id_t who) {
  return 0;
}

weak int __syscall_setpriority(int which, id_t who, int prio) {
  return -EPERM;
}

weak int __syscall_setdomainname(const char *name, size_t len) {
  return -EPERM;
}

weak uid_t __syscall_getuid32(void) {
  return 0;
}

weak gid_t __syscall_getgid32(void) {
  return 0;
}

weak uid_t __syscall_geteuid32(void) {
  return 0;
}

weak gid_t __syscall_getegid32(void) {
  return 0;
}

weak int __syscall_getresuid32(uid_t *ruid, uid_t *euid, uid_t *suid) {
  *ruid = 0;
  *euid = 0;
  *suid = 0;
  return 0;
}

weak int __syscall_getresgid32(gid_t *rgid, gid_t *egid, gid_t *sgid) {
  REPORT(getresgid32);
  *rgid = 0;
  *egid = 0;
  *sgid = 0;
  return 0;
}

weak int __syscall_pause() {
  REPORT(pause);
  return -EINTR; // we can't pause
}

weak int __syscall_madvise(void *addr, size_t length, int advice) {
  REPORT(madvise);
  // advice is welcome, but ignored
  return 0;
}

weak int __syscall_mlock(const void *addr, size_t len) {
  REPORT(mlock);
  return 0;
}

weak int __syscall_munlock(const void *addr, size_t len) {
  REPORT(munlock);
  return 0;
}

weak int __syscall_mprotect(size_t start, size_t len, int prot) {
  REPORT(mprotect);
  return 0; // let's not and say we did
}

weak int __syscall_mremap(void *old_addr, size_t old_size, size_t new_size, int flags, void *new_addr) {
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

weak int __syscall_prlimit64(pid_t pid, int resource, const struct rlimit *new_limit, struct rlimit *old_limit) {
  REPORT(prlimit64);
  if (new_limit) {
    return -EPERM;
  }
  if (old_limit) {
    if (resource == RLIMIT_NOFILE) {
      // See FS.MAX_OPEN_FDS in src/lib/libfs.js
      old_limit->rlim_cur = 4096;
      old_limit->rlim_max = 4096;
    } else if (resource == RLIMIT_STACK) {
      uintptr_t end = emscripten_stack_get_end();
      uintptr_t base = emscripten_stack_get_base();

      old_limit->rlim_cur = base - end;
      // we can not change the stack size, so the maximum is the same as the current
      old_limit->rlim_max = base - end;
    } else {
      // Just report no limits
      old_limit->rlim_cur = RLIM_INFINITY;
      old_limit->rlim_max = RLIM_INFINITY;
    }
  }
  return 0;
}

weak int __syscall_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen, ...) {
  REPORT(setsockopt);
  return -ENOPROTOOPT; // The option is unknown at the level indicated.
}

weak pid_t __syscall_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage) {
  REPORT(wait4);
  return (pid_t)-1;
}

UNIMPLEMENTED(acct, (const char *filename))
UNIMPLEMENTED(mincore, (void *addr, size_t length, unsigned char *vec))
UNIMPLEMENTED(pipe2, (int pipefd[2], int flags))
UNIMPLEMENTED(pselect6, (int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, const struct timespec *ts, const void *mask))
UNIMPLEMENTED(recvmmsg, (int sockfd, struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags, struct timespec *timeout))
UNIMPLEMENTED(sendmmsg, (int sockfd, struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags))
UNIMPLEMENTED(shutdown, (int sockfd, int how, ...))
UNIMPLEMENTED(socketpair, (int domain, int type, int protocol, int fd[2], ...))
