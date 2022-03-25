#include <wasi/api.h>
#include <wasi/wasi-helpers.h>
#include <emscripten/em_macros.h>

// Compile as if we can pass uint64 values directly to the
// host.  Binaryen will take care of splitting any i64 params
// into a pair of i32 values if needed.
#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#ifdef __cplusplus
extern "C" {
#endif

int __syscall_link(long oldpath, long newpath);
int __syscall_chdir(long path);
int __syscall_mknod(long path, long mode, long dev);
int __syscall_chmod(long path, long mode);
int __syscall_getpid(void);
int __syscall_pause(void);
int __syscall_access(long path, long amode);
int __syscall_sync(void);
int __syscall_mkdir(long path, long mode);
int __syscall_rmdir(long path);
int __syscall_dup(long fd);
int __syscall_pipe(long fd);
int __syscall_acct(long filename);
int __syscall_ioctl(long fd, long request, ...);
int __syscall_setpgid(long pid, long gpid);
int __syscall_umask(long mask);
int __syscall_getppid(void);
int __syscall_getpgrp(void);
int __syscall_setsid(void);
int __syscall_setrlimit(long resource, long limit);
int __syscall_getrusage(long who, long usage);
int __syscall_symlink(long target, long linkpath);
int __syscall_munmap(long addr, long len);
int __syscall_fchmod(long fd, long mode);
int __syscall_getpriority(long which, long who);
int __syscall_setpriority(long which, long who, long prio);
int __syscall_socketcall(long call, long args);
int __syscall_setitimer(long which, long new_value, long old_value);
int __syscall_getitimer(long which, long old_value);
int __syscall_wait4(long pid, long wstatus, long options, long rusage);
int __syscall_setdomainname(long name, long size);
int __syscall_uname(long buf);
int __syscall_mprotect(long addr, long len, long size);
int __syscall_getpgid(long pid);
int __syscall_fchdir(long fd);
int __syscall__newselect(long nfds, long readfds, long writefds, long exceptfds, long timeout);
int __syscall_msync(long addr, long len, long flags);
int __syscall_getsid(long pid);
int __syscall_fdatasync(long fd);
int __syscall_mlock(long addr, long len);
int __syscall_munlock(long addr, long len);
int __syscall_mlockall(long flags);
int __syscall_munlockall(void);
int __syscall_mremap(long old_addr, long old_size, long new_size, long flags, long new_addr);
int __syscall_poll(long fds, long nfds, long timeout);
int __syscall_getcwd(long buf, long size);
int __syscall_ugetrlimit(long resource, long rlim);
int __syscall_mmap2(long addr, long len, long prot, long flags, long fd, long off);
int __syscall_truncate64(long path, uint64_t length);
int __syscall_ftruncate64(long fd, uint64_t length);
int __syscall_stat64(long path, long buf);
int __syscall_lstat64(long path, long buf);
int __syscall_fstat64(long fd, long buf);
int __syscall_getuid32(void);
int __syscall_getgid32(void);
int __syscall_geteuid32(void);
int __syscall_getegid32(void);
int __syscall_setreuid32(long ruid, long euid);
int __syscall_setregid32(long rgid, long egid);
int __syscall_getgroups32(long size, long list);
int __syscall_fchown32(long fd, long owner, long group);
int __syscall_setresuid32(long ruid, long euid, long suid);
int __syscall_getresuid32(long ruid, long euid, long suid);
int __syscall_setresgid32(long rgid, long egid, long sgid);
int __syscall_getresgid32(long rgid, long egid, long sgid);
int __syscall_setuid32(long uid);
int __syscall_setgid32(long uid);
int __syscall_mincore(long addr, long length, long vec);
int __syscall_madvise(long addr, long length, long advice);
int __syscall_getdents64(long fd, long dirp, long count);
int __syscall_fcntl64(long fd, long cmd, ...);
int __syscall_statfs64(long path, long size, long buf);
int __syscall_fstatfs64(long fd, long size, long buf);
int __syscall_fadvise64(long fd, uint64_t base, uint64_t len, long advice);
int __syscall_openat(long dirfd, long path, long flags, ...); // mode is optional
int __syscall_mkdirat(long dirfd, long path, long mode);
int __syscall_mknodat(long dirfd, long path, long mode, long dev);
int __syscall_fchownat(long dirfd, long path, long owner, long group, long flags);
int __syscall_newfstatat(long dirfd, long path, long buf, long flags);
int __syscall_unlinkat(long dirfd, long path, long flags);
int __syscall_renameat(long olddirfd, long oldpath, long newdirfd, long newpath);
int __syscall_linkat(long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
int __syscall_symlinkat(long target, long newdirfd, long linkpath);
int __syscall_readlinkat(long dirfd, long path, long buf, long bufsize);
int __syscall_fchmodat(long dirfd, long path, long mode, ...);
int __syscall_faccessat(long dirfd, long path, long amode, long flags);
int __syscall_pselect6(long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
int __syscall_utimensat(long dirfd, long path, long times, long flags);
int __syscall_fallocate(long fd, long mode, uint64_t off, uint64_t len);
int __syscall_dup3(long fd, long suggestfd, long flags);
int __syscall_pipe2(long fds, long flags);
int __syscall_recvmmsg(long sockfd, long msgvec, long vlen, long flags, ...);
int __syscall_prlimit64(long pid, long resource, long new_limit, long old_limit);
int __syscall_sendmmsg(long sockfd, long msgvec, long vlen, long flags, ...);
int __syscall_socket(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_socketpair(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_bind(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_connect(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_listen(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_accept4(long sockfd, long addr, long addrlen, long flags, long dummy1, long dummy2);
int __syscall_getsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_setsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_getsockname(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_getpeername(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_sendto(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_sendmsg(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_recvfrom(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_recvmsg(long sockfd, long level, long optname, long optval, long optlen, long dummy);
int __syscall_shutdown(long sockfd, long level, long optname, long optval, long optlen, long dummy);

#ifdef __cplusplus
}
#endif
