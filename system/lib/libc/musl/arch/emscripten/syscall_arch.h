#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include <sys/types.h>

#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

// Compile as if we can pass uint64 values directly to the
// host.  Binaryen will take care of splitting any i64 params
// into a pair of i32 values if needed.
#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#ifdef __cplusplus
extern "C" {
#endif

int __syscall_chdir(const char *path);
int __syscall_mknod(const char *path, mode_t mode, dev_t dev);
int __syscall_chmod(const char *path, mode_t mode);
pid_t __syscall_getpid(void);
int __syscall_pause(void);
int __syscall_access(const char *path, int amode);
int __syscall_sync(void);
int __syscall_rmdir(const char *path);
int __syscall_dup(int fd);
int __syscall_pipe(int fd[2]);
int __syscall_acct(const char *filename);
int __syscall_ioctl(int fd, int request, ...);
int __syscall_setpgid(pid_t pid, pid_t gpid);
mode_t __syscall_umask(mode_t mask);
pid_t __syscall_getppid(void);
pid_t __syscall_getpgrp(void);
pid_t __syscall_setsid(void);
int __syscall_setrlimit(int resource, const void *rlp);
int __syscall_getrusage(int who, void *usage);
int __syscall_symlink(const char *target, const char *linkpath);
int __syscall_munmap(void *addr, size_t len);
int __syscall_fchmod(int fd, mode_t mode);
int __syscall_getpriority(int which, id_t who);
int __syscall_setpriority(int which, id_t who, int prio);
int __syscall_socketcall(int call, long args[6]);
pid_t __syscall_wait4(pid_t pid, int *wstatus, int options, void *rusage);
int __syscall_setdomainname(const char *name, size_t len);
int __syscall_uname(void *buf);
int __syscall_mprotect(size_t start, size_t len, int prot);
pid_t __syscall_getpgid(pid_t pid);
int __syscall_fchdir(int fd);
int __syscall__newselect(int nfds, void *readfds, void *writefds, void *exceptfds, void *timeout);
int __syscall_msync(void *addr, size_t len, int flags);
pid_t __syscall_getsid(pid_t pid);
int __syscall_fdatasync(int fd);
int __syscall_mlock(const void *addr, size_t len);
int __syscall_munlock(const void *addr, size_t len);
int __syscall_mlockall(int flags);
int __syscall_munlockall(void);
int __syscall_mremap(void *old_addr, size_t old_size, size_t new_size, int flags, void *new_addr);
int __syscall_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int __syscall_getcwd(char *buf, size_t size);
int __syscall_ugetrlimit(int resource, void *rlim);
intptr_t __syscall_mmap2(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int __syscall_truncate64(const char *path, off_t length);
int __syscall_ftruncate64(int fd, off_t length);
int __syscall_stat64(const char *path, struct stat *buf);
int __syscall_lstat64(const char *path, struct stat *buf);
int __syscall_fstat64(int fd, struct stat *buf);
uid_t __syscall_getuid32(void);
gid_t __syscall_getgid32(void);
uid_t __syscall_geteuid32(void);
gid_t __syscall_getegid32(void);
int __syscall_setreuid32(uid_t ruid, uid_t euid);
int __syscall_setregid32(gid_t rgid, gid_t egid);
int __syscall_getgroups32(int count, gid_t list[]);
int __syscall_fchown32(int fd, uid_t owner, gid_t group);
int __syscall_setresuid32(uid_t ruid, uid_t euid, uid_t suid);
int __syscall_getresuid32(uid_t *ruid, uid_t *euid, uid_t *suid);
int __syscall_setresgid32(gid_t rgid, gid_t egid, gid_t sgid);
int __syscall_getresgid32(gid_t *rgid, gid_t *egid, gid_t *sgid);
int __syscall_setuid32(uid_t uid);
int __syscall_setgid32(gid_t gid);
int __syscall_mincore(void *addr, size_t length, unsigned char *vec);
int __syscall_madvise(void *addr, size_t length, int advice);
int __syscall_getdents64(int fd, void *dirp, size_t count);
int __syscall_fcntl64(int fd, int cmd, ...);
int __syscall_statfs64(const char *path, size_t size, struct statfs *buf);
int __syscall_fstatfs64(int fd, size_t size, struct statfs *buf);
int __syscall_fadvise64(int fd, off_t offset, off_t len, int advice);
int __syscall_openat(int dirfd, const char *path, int flags, ...); // mode is optional
int __syscall_mkdirat(int dirfd, const char *path, mode_t mode);
int __syscall_mknodat(int dirfd, const char *path, mode_t mode, dev_t dev);
int __syscall_fchownat(int dirfd, const char *path, uid_t owner, gid_t group, int flags);
int __syscall_newfstatat(int dirfd, const char *path, struct stat *buf, int flags);
int __syscall_unlinkat(int dirfd, const char *path, int flags);
int __syscall_renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
int __syscall_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int __syscall_symlinkat(const char *target, int newdirfd, const char *linkpath);
int __syscall_readlinkat(int dirfd, const char *path, char *buf, size_t bufsize);
int __syscall_fchmodat2(int dirfd, const char *path, mode_t mode, int flags);
int __syscall_faccessat(int dirfd, const char *path, int amode, int flags);
int __syscall_pselect6(int nfds, void *rfds, void *wfds, void *efds, void *ts, const void *mask);
int __syscall_utimensat(int dirfd, const char *path, const struct timespec times[2], int flags);
int __syscall_fallocate(int fd, int mode, off_t offset, off_t len);
int __syscall_dup3(int oldfd, int newfd, int flags);
int __syscall_pipe2(int pipefd[2], int flags);
int __syscall_recvmmsg(int sockfd, void *msgvec, unsigned int vlen, int flags, struct timespec *timeout);
int __syscall_prlimit64(pid_t pid, int resource, const void *new_limit, void *old_limit);
int __syscall_sendmmsg(int sockfd, void *msgvec, unsigned int vlen, int flags);
int __syscall_socket(int domain, int type, int protocol, ...);
int __syscall_socketpair(int domain, int type, int protocol, int fd[2], ...);
int __syscall_bind(int sockfd, const void *addr, socklen_t len, ...);
int __syscall_connect(int sockfd, const void *addr, socklen_t len, ...);
int __syscall_listen(int sockfd, int backlog, ...);
int __syscall_accept4(int sockfd, void *addr, socklen_t *len, int flags, ...);
int __syscall_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen, ...);
int __syscall_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen, ...);
int __syscall_getsockname(int sockfd, void *addr, socklen_t *len, ...);
int __syscall_getpeername(int sockfd, void *addr, socklen_t *len, ...);
ssize_t __syscall_sendto(int sockfd, const void *buf, size_t len, int flags, const void *addr, socklen_t alen);
ssize_t __syscall_sendmsg(int sockfd, const void *msg, int flags, ...);
ssize_t __syscall_recvfrom(int sockfd, void *buf, size_t len, int flags, void *addr, socklen_t *alen);
ssize_t __syscall_recvmsg(int sockfd, void *msg, int flags, ...);
int __syscall_shutdown(int sockfd, int how, ...);

#ifdef __cplusplus
}
#endif
