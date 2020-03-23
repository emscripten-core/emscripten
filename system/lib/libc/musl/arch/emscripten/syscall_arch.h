#include <wasi/api.h>
#include <wasi/wasi-helpers.h>
#include <emscripten/emscripten.h>

#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) 0, __SYSCALL_LL_E((x))

#define __SC_socket      1
#define __SC_bind        2
#define __SC_connect     3
#define __SC_listen      4
#define __SC_accept      5
#define __SC_getsockname 6
#define __SC_getpeername 7
#define __SC_socketpair  8
#define __SC_send        9
#define __SC_recv        10
#define __SC_sendto      11
#define __SC_recvfrom    12
#define __SC_shutdown    13
#define __SC_setsockopt  14
#define __SC_getsockopt  15
#define __SC_sendmsg     16
#define __SC_recvmsg     17
#define __SC_accept4     18
#define __SC_recvmmsg    19
#define __SC_sendmmsg    20

#ifdef __cplusplus
extern "C" {
#endif

/* Causes the final import in the wasm binary be named "env.sys_<name>" */
#define SYS_IMPORT(NAME) EM_IMPORT(__sys_##NAME)

long SYS_IMPORT(exit) __syscall1(long exit_code);
long SYS_IMPORT(read) __syscall3(long fd, long buf, long count);
long SYS_IMPORT(write) __syscall4(long fd, long buf, long count);
long SYS_IMPORT(open) __syscall5(long path, long flags, ...); // mode is optional
long SYS_IMPORT(link) __syscall9(long oldpath, long newpath);
long SYS_IMPORT(unlink) __syscall10(long path);
long SYS_IMPORT(chdir) __syscall12(long path);
long SYS_IMPORT(mknod) __syscall14(long path, long mode, long dev);
long SYS_IMPORT(chmod) __syscall15(long path, long mode);
long SYS_IMPORT(getpid) __syscall20(void);
long SYS_IMPORT(pause) __syscall29(void);
long SYS_IMPORT(access) __syscall33(long path, long amode);
long SYS_IMPORT(nice) __syscall34(long inc);
long SYS_IMPORT(sync) __syscall36(void);
long SYS_IMPORT(rename) __syscall38(long old_path, long new_path);
long SYS_IMPORT(mkdir) __syscall39(long path, long mode);
long SYS_IMPORT(rmdir) __syscall40(long path);
long SYS_IMPORT(dup) __syscall41(long fd);
long SYS_IMPORT(pipe) __syscall42(long fd);
long SYS_IMPORT(acct) __syscall51(long filename);
long SYS_IMPORT(ioctl) __syscall54(long fd, long request, ...);
long SYS_IMPORT(setpgid) __syscall57(long pid, long gpid);
long SYS_IMPORT(umask) __syscall60(long mask);
long SYS_IMPORT(dup2) __syscall63(long oldfd, long newfd);
long SYS_IMPORT(getppid) __syscall64(void);
long SYS_IMPORT(getpgrp) __syscall65(void);
long SYS_IMPORT(setsid) __syscall66(void);
long SYS_IMPORT(setrlimit) __syscall75(long resource, long limit);
long SYS_IMPORT(getrusage) __syscall77(long who, long usage);
long SYS_IMPORT(symlink) __syscall83(long target, long linkpath);
long SYS_IMPORT(readlink) __syscall85(long path, long buf, long bufsize);
long SYS_IMPORT(munmap) __syscall91(long addr, long len);
long SYS_IMPORT(fchmod) __syscall94(long fd, long mode);
long SYS_IMPORT(getpriority) __syscall96(long which, long who);
long SYS_IMPORT(setpriority) __syscall97(long which, long who, long prio);
long SYS_IMPORT(socketcall) __syscall102(long call, long args);
long SYS_IMPORT(setitimer) __syscall104(long which, long new_value, long old_value);
long SYS_IMPORT(wait4) __syscall114(long pid, long wstatus, long options, long rusage);
long SYS_IMPORT(setdomainname) __syscall121(long name, long size);
long SYS_IMPORT(uname) __syscall122(long buf);
long SYS_IMPORT(mprotect) __syscall125(long addr, long len, long size);
long SYS_IMPORT(getpgid) __syscall132(long pid);
long SYS_IMPORT(fchdir) __syscall133(long fd);
long SYS_IMPORT(_newselect)
  __syscall142(long nfds, long readfds, long writefds, long exceptfds, long timeout);
long SYS_IMPORT(msync) __syscall144(long addr, long len, long flags);
long SYS_IMPORT(getsid) __syscall147(long pid);
long SYS_IMPORT(fdatasync) __syscall148(long fd);
long SYS_IMPORT(mlock) __syscall150(long addr, long len);
long SYS_IMPORT(munlock) __syscall151(long addr, long len);
long SYS_IMPORT(mlockall) __syscall152(long flags);
long SYS_IMPORT(munlockall) __syscall153(void);
long SYS_IMPORT(mremap)
  __syscall163(long old_addr, long old_size, long new_size, long flags, long new_addr);
long SYS_IMPORT(poll) __syscall168(long fds, long nfds, long timeout);
long SYS_IMPORT(rt_sigqueueinfo) __syscall178(long tgid, long sig, long uinfo);
long SYS_IMPORT(pread64)
  __syscall180(long fd, long buf, long count, long zero, long low, long high);
long SYS_IMPORT(pwrite64)
  __syscall181(long fd, long buf, long count, long zero, long low, long high);
long SYS_IMPORT(getcwd) __syscall183(long buf, long size);
long SYS_IMPORT(ugetrlimit) __syscall191(long resource, long rlim);
long SYS_IMPORT(mmap2) __syscall192(long addr, long len, long prot, long flags, long fd, long off);
long SYS_IMPORT(truncate64) __syscall193(long path, long zero, long low, long high);
long SYS_IMPORT(ftruncate64) __syscall194(long fd, long zero, long low, long high);
long SYS_IMPORT(stat64) __syscall195(long path, long buf);
long SYS_IMPORT(lstat64) __syscall196(long path, long buf);
long SYS_IMPORT(fstat64) __syscall197(long fd, long buf);
long SYS_IMPORT(lchown32) __syscall198(long path, long owner, long group);
long SYS_IMPORT(getuid32) __syscall199(void);
long SYS_IMPORT(getgid32) __syscall200(void);
long SYS_IMPORT(geteuid32) __syscall201(void);
long SYS_IMPORT(getegid32) __syscall202(void);
long SYS_IMPORT(setreuid32) __syscall203(long ruid, long euid);
long SYS_IMPORT(setregid32) __syscall204(long rgid, long egid);
long SYS_IMPORT(getgroups32) __syscall205(long size, long list);
long SYS_IMPORT(fchown32) __syscall207(long fd, long owner, long group);
long SYS_IMPORT(setresuid32) __syscall208(long ruid, long euid, long suid);
long SYS_IMPORT(getresuid32) __syscall209(long ruid, long euid, long suid);
long SYS_IMPORT(setresgid32) __syscall210(long rgid, long egid, long sgid);
long SYS_IMPORT(getresgid32) __syscall211(long rgid, long egid, long sgid);
long SYS_IMPORT(chown32) __syscall212(long path, long owner, long group);
long SYS_IMPORT(setuid32) __syscall213(long uid);
long SYS_IMPORT(setgid32) __syscall214(long uid);
long SYS_IMPORT(mincore) __syscall218(long addr, long length, long vec);
long SYS_IMPORT(madvise1) __syscall219(long addr, long length, long advice);
long SYS_IMPORT(getdents64) __syscall220(long fd, long dirp, long count);
long SYS_IMPORT(fcntl64) __syscall221(long fd, long cmd, ...);
long SYS_IMPORT(exit_group) __syscall252(long status);
long SYS_IMPORT(statfs64) __syscall268(long path, long size, long buf);
long SYS_IMPORT(fstatfs64) __syscall269(long fd, long size, long buf);
long SYS_IMPORT(fadvise64_64)
  __syscall272(long fd, long zero, long low, long high, long low2, long high2, long advice);
long SYS_IMPORT(openat) __syscall295(long dirfd, long path, long flags, ...);
long SYS_IMPORT(mkdirat) __syscall296(long dirfd, long path, long mode);
long SYS_IMPORT(mknodat) __syscall297(long dirfd, long path, long mode, long dev);
long SYS_IMPORT(fchownat) __syscall298(long dirfd, long path, long owner, long group, long flags);
long SYS_IMPORT(fstatat64) __syscall300(long dirfd, long path, long buf, long flags);
long SYS_IMPORT(unlinkat) __syscall301(long dirfd, long path, long flags);
long SYS_IMPORT(renameat) __syscall302(long olddirfd, long oldpath, long newdirfd, long newpath);
long SYS_IMPORT(linkat)
  __syscall303(long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
long SYS_IMPORT(symlinkat) __syscall304(long target, long newdirfd, long linkpath);
long SYS_IMPORT(readlinkat) __syscall305(long dirfd, long path, long bug, long bufsize);
long SYS_IMPORT(fchmodat) __syscall306(long dirfd, long path, long mode, ...);
long SYS_IMPORT(faccessat) __syscall307(long dirfd, long path, long amode, long flags);
long SYS_IMPORT(pselect6) __syscall308(long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
long SYS_IMPORT(utimensat) __syscall320(long dirfd, long path, long times, long flags);
long SYS_IMPORT(fallocate) __syscall324(long fd, long mode, long off_low, long off_high, long len_low, long len_high);
long SYS_IMPORT(dup3) __syscall330(long fd, long suggestfd, long flags);
long SYS_IMPORT(pipe2) __syscall331(long fds, long flags);
long SYS_IMPORT(preadv) __syscall333(long fd, long iov, long iovcnt, long low, long high);
long SYS_IMPORT(pwritev) __syscall334(long fd, long iov, long iovcnt, long low, long high);
long SYS_IMPORT(recvmmsg) __syscall337(long sockfd, long msgvec, long vlen, long flags, ...);
long SYS_IMPORT(prlimit64) __syscall340(long pid, long resource, long new_limit, long old_limit);
long SYS_IMPORT(sendmmsg) __syscall345(long sockfd, long msgvec, long vlen, long flags, ...);

#ifdef __cplusplus
}
#endif
