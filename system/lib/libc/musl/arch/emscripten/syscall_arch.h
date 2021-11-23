#include <wasi/api.h>
#include <wasi/wasi-helpers.h>
#include <emscripten/em_macros.h>

#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) __SYSCALL_LL_E((x))

#ifdef __cplusplus
extern "C" {
#endif

long __syscall_open(long path, long flags, ...); // mode is optional
long __syscall_link(long oldpath, long newpath);
long __syscall_unlink(long path);
long __syscall_chdir(long path);
long __syscall_mknod(long path, long mode, long dev);
long __syscall_chmod(long path, long mode);
long __syscall_getpid(void);
long __syscall_pause(void);
long __syscall_access(long path, long amode);
long __syscall_nice(long inc);
long __syscall_sync(void);
long __syscall_rename(long old_path, long new_path);
long __syscall_mkdir(long path, long mode);
long __syscall_rmdir(long path);
long __syscall_dup(long fd);
long __syscall_pipe(long fd);
long __syscall_acct(long filename);
long __syscall_ioctl(long fd, long request, ...);
long __syscall_setpgid(long pid, long gpid);
long __syscall_umask(long mask);
long __syscall_getppid(void);
long __syscall_getpgrp(void);
long __syscall_setsid(void);
long __syscall_setrlimit(long resource, long limit);
long __syscall_getrusage(long who, long usage);
long __syscall_symlink(long target, long linkpath);
long __syscall_readlink(long path, long buf, long bufsize);
long __syscall_munmap(long addr, long len);
long __syscall_fchmod(long fd, long mode);
long __syscall_getpriority(long which, long who);
long __syscall_setpriority(long which, long who, long prio);
long __syscall_socketcall(long call, long args);
long __syscall_setitimer(long which, long new_value, long old_value);
long __syscall_getitimer(long which, long old_value);
long __syscall_wait4(long pid, long wstatus, long options, long rusage);
long __syscall_setdomainname(long name, long size);
long __syscall_uname(long buf);
long __syscall_mprotect(long addr, long len, long size);
long __syscall_getpgid(long pid);
long __syscall_fchdir(long fd);
long __syscall__newselect(long nfds, long readfds, long writefds, long exceptfds, long timeout);
long __syscall_msync(long addr, long len, long flags);
long __syscall_getsid(long pid);
long __syscall_fdatasync(long fd);
long __syscall_mlock(long addr, long len);
long __syscall_munlock(long addr, long len);
long __syscall_mlockall(long flags);
long __syscall_munlockall(void);
long __syscall_mremap(long old_addr, long old_size, long new_size, long flags, long new_addr);
long __syscall_poll(long fds, long nfds, long timeout);
long __syscall_getcwd(long buf, long size);
long __syscall_ugetrlimit(long resource, long rlim);
long __syscall_mmap2(long addr, long len, long prot, long flags, long fd, long off);
long __syscall_truncate64(long path, long low, long high);
long __syscall_ftruncate64(long fd, long low, long high);
long __syscall_stat64(long path, long buf);
long __syscall_lstat64(long path, long buf);
long __syscall_fstat64(long fd, long buf);
long __syscall_lchown32(long path, long owner, long group);
long __syscall_getuid32(void);
long __syscall_getgid32(void);
long __syscall_geteuid32(void);
long __syscall_getegid32(void);
long __syscall_setreuid32(long ruid, long euid);
long __syscall_setregid32(long rgid, long egid);
long __syscall_getgroups32(long size, long list);
long __syscall_fchown32(long fd, long owner, long group);
long __syscall_setresuid32(long ruid, long euid, long suid);
long __syscall_getresuid32(long ruid, long euid, long suid);
long __syscall_setresgid32(long rgid, long egid, long sgid);
long __syscall_getresgid32(long rgid, long egid, long sgid);
long __syscall_chown32(long path, long owner, long group);
long __syscall_setuid32(long uid);
long __syscall_setgid32(long uid);
long __syscall_mincore(long addr, long length, long vec);
long __syscall_madvise(long addr, long length, long advice);
long __syscall_getdents64(long fd, long dirp, long count);
long __syscall_fcntl64(long fd, long cmd, ...);
long __syscall_statfs64(long path, long size, long buf);
long __syscall_fstatfs64(long fd, long size, long buf);
long __syscall_fadvise64_64(long fd, long low, long high, long low2, long high2, long advice);
long __syscall_openat(long dirfd, long path, long flags, ...);
long __syscall_mkdirat(long dirfd, long path, long mode);
long __syscall_mknodat(long dirfd, long path, long mode, long dev);
long __syscall_fchownat(long dirfd, long path, long owner, long group, long flags);
long __syscall_fstatat64(long dirfd, long path, long buf, long flags);
long __syscall_unlinkat(long dirfd, long path, long flags);
long __syscall_renameat(long olddirfd, long oldpath, long newdirfd, long newpath);
long __syscall_linkat(long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
long __syscall_symlinkat(long target, long newdirfd, long linkpath);
long __syscall_readlinkat(long dirfd, long path, long bug, long bufsize);
long __syscall_fchmodat(long dirfd, long path, long mode, ...);
long __syscall_faccessat(long dirfd, long path, long amode, long flags);
long __syscall_pselect6(long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
long __syscall_utimensat(long dirfd, long path, long times, long flags);
long __syscall_fallocate(long fd, long mode, long off_low, long off_high, long len_low, long len_high);
long __syscall_dup3(long fd, long suggestfd, long flags);
long __syscall_pipe2(long fds, long flags);
long __syscall_recvmmsg(long sockfd, long msgvec, long vlen, long flags, ...);
long __syscall_prlimit64(long pid, long resource, long new_limit, long old_limit);
long __syscall_sendmmsg(long sockfd, long msgvec, long vlen, long flags, ...);
long __syscall_socket(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_socketpair(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_bind(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_connect(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_listen(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_accept4(long sockfd, long addr, long addrlen, long flags, long dummy1, long dummy2);
long __syscall_getsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_setsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_getsockname(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_getpeername(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_sendto(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_sendmsg(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_recvfrom(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_recvmsg(long sockfd, long level, long optname, long optval, long optlen, long dummy);
long __syscall_shutdown(long sockfd, long level, long optname, long optval, long optlen, long dummy);

#ifdef __cplusplus
}
#endif
