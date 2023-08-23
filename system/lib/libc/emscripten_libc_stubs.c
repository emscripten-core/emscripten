/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Fake/stub implemenations of libc functions.
 * See emscripten_syscall_stubs.c for fake/stub implemenations of syscalls.
 */

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef weak
#define weak __attribute__((__weak__))
#endif

// ==========================================================================
// sys/wait.h
// ==========================================================================

weak int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
  errno = ECHILD;
  return -1;
}

// ==========================================================================
// sys/times.h
// ==========================================================================

clock_t times(struct tms *buf) {
  // clock_t times(struct tms *buffer);
  // http://pubs.opengroup.org/onlinepubs/009695399/functions/times.html
  // NOTE: This is fake, since we can't calculate real CPU time usage in JS.
  if (buf) {
    memset(buf, 0, sizeof(*buf));
  }
  return 0;
}

struct tm *getdate(const char *string) {
  // struct tm *getdate(const char *string);
  // http://pubs.opengroup.org/onlinepubs/009695399/functions/getdate.html
  // TODO: Implement.
  return 0;
}

weak int stime(const time_t *t) {
  errno = EPERM;
  return -1;
}

weak int clock_getcpuclockid(pid_t pid, clockid_t *clockid) {
  if (pid < 0) {
    return ESRCH;
  }
  if (pid != 0 && pid != getpid()) {
    return ENOSYS;
  }
  if (clockid) {
    *clockid = CLOCK_PROCESS_CPUTIME_ID;
  }
  return 0;
}

// ==========================================================================
// pwd.h
// ==========================================================================

struct passwd *getpwnam(const char *name) {
  errno = ENOENT;
  return 0;
}

struct passwd *getpwuid(uid_t uid) {
  errno = ENOENT;
  return 0;
}

weak int getpwnam_r(const char *name, struct passwd *pwd,
               char *buf, size_t buflen, struct passwd **result) {
  return ENOENT;
}

weak int getpwuid_r(uid_t uid, struct passwd *pwd,
                      char *buf, size_t buflen, struct passwd **result) {
  return ENOENT;
}

weak void setpwent(void) {
}

weak void endpwent(void) {
}

struct passwd *getpwent(void) {
  errno = EIO;
  return NULL;
}

// ==========================================================================
// grp.h
// ==========================================================================

weak struct group *getgrnam(const char *name) {
  errno = ENOENT;
  return 0;
}

weak struct group *getgrgid(gid_t gid) {
  errno = ENOENT;
  return 0;
}

weak int getgrnam_r(const char *name, struct group *grp,
               char *buf, size_t buflen, struct group **result) {
  return ENOENT;
}

weak int getgrgid_r(gid_t gid, struct group *grp,
               char *buf, size_t buflen, struct group **result) {
  return ENOENT;
}

weak struct group *getgrent(void) {
  errno = EIO;
  return NULL;
}

weak void endgrent(void) {
}

weak void setgrent(void) {
}

// ==========================================================================
// sys/file.h
// ==========================================================================

weak int flock(int fd, int operation) {
  // int flock(int fd, int operation);
  // Pretend to succeed
  return 0;
}

weak int chroot(const char *path) {
  // int chroot(const char *path);
  // http://pubs.opengroup.org/onlinepubs/7908799/xsh/chroot.html
  errno = EACCES;
  return -1;
}

weak int execve(const char *pathname, char *const argv[],
           char *const envp[]) {
  // int execve(const char *pathname, char *const argv[],
  //            char *const envp[]);
  // http://pubs.opengroup.org/onlinepubs/009695399/functions/exec.html
  // We don't support executing external code.
  errno = ENOEXEC;
  return -1;
}

weak pid_t fork(void) {
  // pid_t fork(void);
  // http://pubs.opengroup.org/onlinepubs/000095399/functions/fork.html
  // We don't support multiple processes.
  errno = ENOSYS;
  return -1;
}

weak pid_t vfork(void) {
  errno = ENOSYS;
  return -1;
}

weak int posix_spawn(pid_t *pid, const char *path,
                       const posix_spawn_file_actions_t *file_actions,
                       const posix_spawnattr_t *attrp,
                       char *const argv[], char *const envp[]) {
  errno = ENOSYS;
  return -1;
}

// ==========================================================================
// stdio.h
// ==========================================================================

weak FILE *popen(const char *command, const char *type) {
  errno = ENOSYS;
  return NULL;
}

weak int pclose(FILE *stream) {
  errno = ENOSYS;
  return -1;
}

weak int setgroups(size_t size, const gid_t *list) {
  // int setgroups(int ngroups, const gid_t *gidset);
  // https://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man2/setgroups.2.html
  if (size < 1 || size > sysconf(_SC_NGROUPS_MAX)) {
    errno = EINVAL;
    return -1;
  }
  // We have just one process/user/group, so it makes no sense to set groups.
  errno = EPERM;
  return -1;
}

weak int sigaltstack(const stack_t *restrict ss, stack_t *restrict old_ss) {
  errno = ENOSYS;
  return -1;
}

// ==========================================================================
// dlfcn.h
// ==========================================================================

#ifndef EMSCRIPTEN_DYNAMIC_LINKING
void __dl_seterr(const char*, ...);

weak void *__dlsym(void *restrict p, const char *restrict s, void *restrict ra) {
  __dl_seterr("dynamic linking not enabled");
  return NULL;
}

weak void* dlopen(const char* file, int flags) {
  __dl_seterr("dynamic linking not enabled");
  return NULL;
}
#endif

// ==========================================================================
// stdlib.h
// ==========================================================================

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

weak int getloadavg(double loadavg[], int nelem) {
  // http://linux.die.net/man/3/getloadavg
  int limit = MIN(nelem, 3);
  for (int i = 0; i < limit; i++) {
    loadavg[i] = 0.1;
  }
  return limit;
}
