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

int __syscall_chdir(intptr_t path);
int __syscall_mknod(intptr_t path, int mode, int dev);
int __syscall_chmod(intptr_t path, int mode);
int __syscall_getpid(void);
int __syscall_pause(void);
int __syscall_access(intptr_t path, int amode);
int __syscall_sync(void);
int __syscall_rmdir(intptr_t path);
int __syscall_dup(int fd);
int __syscall_pipe(intptr_t fd);
int __syscall_acct(intptr_t filename);
int __syscall_ioctl(int fd, int request, ...);
int __syscall_setpgid(int pid, int gpid);
int __syscall_umask(int mask);
int __syscall_getppid(void);
int __syscall_getpgrp(void);
int __syscall_setsid(void);
int __syscall_setrlimit(int resource, intptr_t limit);
int __syscall_getrusage(int who, intptr_t usage);
int __syscall_symlink(intptr_t target, intptr_t linkpath);
int __syscall_munmap(intptr_t addr, size_t len);
int __syscall_fchmod(int fd, int mode);
int __syscall_getpriority(int which, int who);
int __syscall_setpriority(int which, int who, int prio);
int __syscall_socketcall(int call, intptr_t args);
int __syscall_wait4(int pid, intptr_t wstatus, int options, int rusage);
int __syscall_setdomainname(intptr_t name, size_t size);
int __syscall_uname(intptr_t buf);
int __syscall_mprotect(size_t addr, size_t len, int prot);
int __syscall_getpgid(int pid);
int __syscall_fchdir(int fd);
int __syscall__newselect(int nfds, intptr_t readfds, intptr_t writefds, intptr_t exceptfds, intptr_t timeout);
int __syscall_msync(intptr_t addr, size_t len, int flags);
int __syscall_getsid(int pid);
int __syscall_fdatasync(int fd);
int __syscall_mlock(intptr_t addr, size_t len);
int __syscall_munlock(intptr_t addr, size_t len);
int __syscall_mlockall(int flags);
int __syscall_munlockall(void);
int __syscall_mremap(intptr_t old_addr, size_t old_size, size_t new_size, int flags, intptr_t new_addr);
int __syscall_poll(intptr_t fds, int nfds, int timeout);
int __syscall_getcwd(intptr_t buf, size_t size);
int __syscall_ugetrlimit(int resource, intptr_t rlim);
intptr_t __syscall_mmap2(intptr_t addr, size_t len, int prot, int flags, int fd, off_t offset);
int __syscall_truncate64(intptr_t path, off_t length);
int __syscall_ftruncate64(int fd, off_t length);
int __syscall_stat64(intptr_t path, intptr_t buf);
int __syscall_lstat64(intptr_t path, intptr_t buf);
int __syscall_fstat64(int fd, intptr_t buf);
int __syscall_getuid32(void);
int __syscall_getgid32(void);
int __syscall_geteuid32(void);
int __syscall_getegid32(void);
int __syscall_setreuid32(int ruid, int euid);
int __syscall_setregid32(int rgid, int egid);
int __syscall_getgroups32(int size, intptr_t list);
int __syscall_fchown32(int fd, int owner, int group);
int __syscall_setresuid32(int ruid, int euid, int suid);
int __syscall_getresuid32(intptr_t ruid, intptr_t euid, intptr_t suid);
int __syscall_setresgid32(int rgid, int egid, int sgid);
int __syscall_getresgid32(intptr_t rgid, intptr_t egid, intptr_t sgid);
int __syscall_setuid32(int uid);
int __syscall_setgid32(int uid);
int __syscall_mincore(intptr_t addr, size_t length, intptr_t vec);
int __syscall_madvise(intptr_t addr, size_t length, int advice);
int __syscall_getdents64(int fd, intptr_t dirp, size_t count);
int __syscall_fcntl64(int fd, int cmd, ...);
int __syscall_statfs64(intptr_t path, size_t size, intptr_t buf);
int __syscall_fstatfs64(int fd, size_t size, intptr_t buf);
int __syscall_fadvise64(int fd, off_t offset, off_t length, int advice);
int __syscall_openat(int dirfd, intptr_t path, int flags, ...); // mode is optional
int __syscall_mkdirat(int dirfd, intptr_t path, int mode);
int __syscall_mknodat(int dirfd, intptr_t path, int mode, int dev);
int __syscall_fchownat(int dirfd, intptr_t path, int owner, int group, int flags);
int __syscall_newfstatat(int dirfd, intptr_t path, intptr_t buf, int flags);
int __syscall_unlinkat(int dirfd, intptr_t path, int flags);
int __syscall_renameat(int olddirfd, intptr_t oldpath, int newdirfd, intptr_t newpath);
int __syscall_linkat(int olddirfd, intptr_t oldpath, int newdirfd, intptr_t newpath, int flags);
int __syscall_symlinkat(intptr_t target, int newdirfd, intptr_t linkpath);
int __syscall_readlinkat(int dirfd, intptr_t path, intptr_t buf, size_t bufsize);
int __syscall_fchmodat2(int dirfd, intptr_t path, int mode, int flags);
int __syscall_faccessat(int dirfd, intptr_t path, int amode, int flags);
int __syscall_pselect6(int nfds, intptr_t readfds, intptr_t writefds, intptr_t exceptfds, intptr_t timeout, intptr_t sigmaks);
int __syscall_utimensat(int dirfd, intptr_t path, intptr_t times, int flags);
int __syscall_fallocate(int fd, int mode, off_t offset, off_t len);
int __syscall_dup3(int fd, int suggestfd, int flags);
int __syscall_pipe2(intptr_t fds, int flags);
int __syscall_recvmmsg(int sockfd, intptr_t msgvec, size_t vlen, int flags, ...);
int __syscall_prlimit64(int pid, int resource, intptr_t new_limit, intptr_t old_limit);
int __syscall_sendmmsg(int sockfd, intptr_t msgvec, size_t vlen, int flags, ...);
int __syscall_socket(int domain, int type, int protocol, int dummy1, int dummy2, int dummy3);
int __syscall_socketpair(int domain, int type, int protocol, intptr_t fds, int dummy, int dummy2);
int __syscall_bind(int sockfd, intptr_t addr, size_t alen, int dummy, int dummy2, int dummy3);
int __syscall_connect(int sockfd, intptr_t addr, size_t len, int dummy, int dummy2, int dummy3);
int __syscall_listen(int sockfd, int backlock, int dummy1, int dummy2, int dummy3, int dummy4);
int __syscall_accept4(int sockfd, intptr_t addr, intptr_t addrlen, int flags, int dummy1, int dummy2);
int __syscall_getsockopt(int sockfd, int level, int optname, intptr_t optval, intptr_t optlen, int dummy);
int __syscall_setsockopt(int sockfd, int level, int optname, intptr_t optval, size_t optlen, int dummy);
int __syscall_getsockname(int sockfd, intptr_t addr, intptr_t len, int dummy, int dummy2, int dummy3);
int __syscall_getpeername(int sockfd, intptr_t addr, intptr_t len, int dummy, int dummy2, int dummy3);
int __syscall_sendto(int sockfd, intptr_t msg, size_t len, int flags, intptr_t addr, size_t alen);
int __syscall_sendmsg(int sockfd, intptr_t msg , int flags, intptr_t addr, size_t alen, int dummy);
int __syscall_recvfrom(int sockfd, intptr_t msg, size_t len, int flags, intptr_t addr, intptr_t alen);
int __syscall_recvmsg(int sockfd, intptr_t msg, int flags, int dummy, int dummy2, int dummy3);
int __syscall_shutdown(int sockfd, int how, int dummy, int dummy2, int dummy3, int dummy4);

#ifdef __cplusplus
}
#endif
