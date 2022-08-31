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

// ==========================================================================
// sys/wait.h
// ==========================================================================

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
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

int stime(const time_t *t) {
  errno = EPERM;
  return -1;
}

int clock_getcpuclockid(pid_t pid, clockid_t *clockid) {
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


// pwd.h

struct passwd *getpwnam(const char *name) {
  errno = ENOENT;
  return 0;
}

struct passwd *getpwuid(uid_t uid) {
  errno = ENOENT;
  return 0;
}

int getpwnam_r(const char *name, struct passwd *pwd,
               char *buf, size_t buflen, struct passwd **result) {
  return ENOENT;
}

int getpwuid_r(uid_t uid, struct passwd *pwd,
                      char *buf, size_t buflen, struct passwd **result) {
  return ENOENT;
}

void setpwent(void) {
}

void endpwent(void) {
}

struct passwd *getpwent(void) {
  errno = EIO;
  return NULL;
}

// grp.h

struct group *getgrnam(const char *name) {
  errno = ENOENT;
  return 0;
}

struct group *getgrgid(gid_t gid) {
  errno = ENOENT;
  return 0;
}

int getgrnam_r(const char *name, struct group *grp,
               char *buf, size_t buflen, struct group **result) {
  return ENOENT;
}

int getgrgid_r(gid_t gid, struct group *grp,
               char *buf, size_t buflen, struct group **result) {
  return ENOENT;
}

struct group *getgrent(void) {
  errno = EIO;
  return NULL;
}

void endgrent(void) {
}

void setgrent(void) {
}

// ==========================================================================
// sys/file.h
// ==========================================================================

int flock(int fd, int operation) {
  // int flock(int fd, int operation);
  // Pretend to succeed
  return 0;
}

int chroot(const char *path) {
  // int chroot(const char *path);
  // http://pubs.opengroup.org/onlinepubs/7908799/xsh/chroot.html
  errno = EACCES;
  return -1;
}

int execve(const char *pathname, char *const argv[],
           char *const envp[]) {
  // int execve(const char *pathname, char *const argv[],
  //            char *const envp[]);
  // http://pubs.opengroup.org/onlinepubs/009695399/functions/exec.html
  // We don't support executing external code.
  errno = ENOEXEC;
  return -1;
}

pid_t fork(void) {
  // pid_t fork(void);
  // http://pubs.opengroup.org/onlinepubs/000095399/functions/fork.html
  // We don't support multiple processes.
  errno = ENOSYS;
  return -1;
}

pid_t vfork(void) {
  errno = ENOSYS;
  return -1;
}

int posix_spawn(pid_t *pid, const char *path,
                       const posix_spawn_file_actions_t *file_actions,
                       const posix_spawnattr_t *attrp,
                       char *const argv[], char *const envp[]) {
  errno = ENOSYS;
  return -1;
}

FILE *popen(const char *command, const char *type) {
  errno = ENOSYS;
  return NULL;
}

int pclose(FILE *stream) {
  errno = ENOSYS;
  return -1;
}

int setgroups(size_t size, const gid_t *list) {
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

int sigaltstack(const stack_t *restrict ss, stack_t *restrict old_ss) {
  errno = ENOSYS;
  return -1;
}
