#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

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

// static syscalls. we must have one non-variadic argument before the rest due to ISO C.

#ifdef __cplusplus
extern "C" {
#endif

long __syscall1(int which, long exit_code);
long __syscall3(int which, long fd, long buf, long count);
long __syscall4(int which, long fd, long buf, long count);
long __syscall5(int which, long path, long flags, ...);  // mode is optional
long __syscall9(int which, long oldpath, long newpath);
long __syscall10(int which, long path);
long __syscall12(int which, long path);
long __syscall14(int which, long path, long mode, long dev);
long __syscall15(int which, long path, long mode);
long __syscall20(int which);
long __syscall29(int which);
long __syscall33(int which, long path, long amode);
long __syscall34(int which, long inc);
long __syscall36(int which);
long __syscall38(int which, long old_path, long new_path);
long __syscall39(int which, long path, long mode);
long __syscall40(int which, long path);
long __syscall41(int which, long fd);
long __syscall42(int which, long fd);
long __syscall51(int which, long filename);
long __syscall54(int which, long fd, long request, ...);
long __syscall57(int which, long pid, long gpid);
long __syscall60(int which, long mask);
long __syscall63(int which, long oldfd, long newfd);
long __syscall64(int which);
long __syscall65(int which);
long __syscall66(int which);
long __syscall75(int which, long resource, long limit);
long __syscall77(int which, long who, long usage);
long __syscall83(int which, long target, long linkpath);
long __syscall85(int which, long path, long buf, long bufsize);
long __syscall91(int which, long addr, long len);
long __syscall94(int which, long fd, long mode);
long __syscall96(int which, long which_, long who);
long __syscall97(int which, long which_, long who, long prio);
long __syscall102(int which, long call, long args);
long __syscall104(int which, long which_, long new_value, long old_value);
long __syscall114(int which, long pid, long wstatus, long options, long rusage);
long __syscall121(int which, long name, long size);
long __syscall122(int which, long buf);
long __syscall125(int which, long addr, long len, long size);
long __syscall132(int which, long pid);
long __syscall133(int which, long fd);
long __syscall142(int which, long nfds, long readfds, long writefds, long exceptfds, long timeout);
long __syscall144(int which, long addr, long len, long flags);
long __syscall147(int which, long pid);
long __syscall148(int which, long fd);
long __syscall150(int which, long addr, long len);
long __syscall151(int which, long addr, long len);
long __syscall152(int which, long flags);
long __syscall153(int which);
long __syscall163(int which, long old_addr, long old_size, long new_size, long flags, long new_addr);
long __syscall168(int which, long fds, long nfds, long timeout);
long __syscall178(int which, long tgid, long sig, long uinfo);
long __syscall180(int which, long fd, long buf, long count, long zero, long low, long high);
long __syscall181(int which, long fd, long buf, long count, long zero, long low, long high);
long __syscall183(int which, long buf, long size);
long __syscall191(int which, long resource, long rlim);
long __syscall192(int which, long addr, long len, long prot, long flags, long fd, long off);
long __syscall193(int which, long path, long zero, long low, long high);
long __syscall194(int which, long fd, long zero, long low, long high);
long __syscall195(int which, long path, long buf);
long __syscall196(int which, long path, long buf);
long __syscall197(int which, long fd, long buf);
long __syscall198(int which, long path, long owner, long group);
long __syscall199(int which);
long __syscall200(int which);
long __syscall201(int which);
long __syscall202(int which);
long __syscall203(int which, long ruid, long euid);
long __syscall204(int which, long rgid, long egid);
long __syscall205(int which, long size, long list);
long __syscall207(int which, long fd, long owner, long group);
long __syscall208(int which, long ruid, long euid, long suid);
long __syscall209(int which, long ruid, long euid, long suid);
long __syscall210(int which, long rgid, long egid, long sgid);
long __syscall211(int which, long rgid, long egid, long sgid);
long __syscall212(int which, long path, long owner, long group);
long __syscall213(int which, long uid);
long __syscall214(int which, long uid);
long __syscall218(int which, long addr, long length, long vec);
long __syscall219(int which, long addr, long length, long advice);
long __syscall220(int which, long fd, long dirp, long count);
long __syscall221(int which, long fd, long cmd, ...);
long __syscall252(int which, long status);
long __syscall268(int which, long path, long size, long buf);
long __syscall269(int which, long fd, long size, long buf);
long __syscall272(int which, long fd, long zero, long low, long high, long low2, long high2, long advice);
long __syscall295(int which, long dirfd, long path, long flags, ...);
long __syscall296(int which, long dirfd, long path, long mode);
long __syscall297(int which, long dirfd, long path, long mode, long dev);
long __syscall298(int which, long dirfd, long path, long owner, long group, long flags);
long __syscall299(int which, long dirfd, long path, long times);
long __syscall300(int which, long dirfd, long path, long buf, long flags);
long __syscall301(int which, long dirfd, long path, long flags);
long __syscall302(int which, long olddirfd, long oldpath, long newdirfd, long newpath);
long __syscall303(int which, long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
long __syscall304(int which, long target, long newdirfd, long linkpath);
long __syscall305(int which, long dirfd, long path, long bug, long bufsize);
long __syscall306(int which, long dirfd, long path, long mode, ...);
long __syscall307(int which, long dirfd, long path, long amode, long flags);
long __syscall308(int which, long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
long __syscall320(int which, long dirfd, long path, long times, long flags);
long __syscall324(int which, long fd, long mode, long off_low, long off_high, long len_low, long len_high);
long __syscall330(int which, long fd, long suggestfd, long flags);
long __syscall331(int which, long fds, long flags);
long __syscall333(int which, long fd, long iov, long iovcnt, long low, long high);
long __syscall334(int which, long fd, long iov, long iovcnt, long low, long high);
long __syscall337(int which, long sockfd, long msgvec, long vlen, long flags, ...);
long __syscall340(int which, long pid, long resource, long new_limit, long old_limit);
long __syscall345(int which, long sockfd, long msgvec, long vlen, long flags, ...);

#ifdef __cplusplus
}
#endif
