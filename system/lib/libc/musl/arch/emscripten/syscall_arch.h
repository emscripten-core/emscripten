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

#ifdef __cplusplus
extern "C" {
#endif
 
long __syscall1(long exit_code);
long __syscall3(long fd, long buf, long count);
long __syscall4(long fd, long buf, long count);
long __syscall5(long path, long flags, ...);  // mode is optional
long __syscall9(long oldpath, long newpath);
long __syscall10(long path);
long __syscall12(long path);
long __syscall14(long path, long mode, long dev);
long __syscall15(long path, long mode);
long __syscall20();
long __syscall29();
long __syscall33(long path, long amode);
long __syscall34(long inc);
long __syscall36();
long __syscall38(long old_path, long new_path);
long __syscall39(long path, long mode);
long __syscall40(long path);
long __syscall41(long fd);
long __syscall42(long fd);
long __syscall51(long filename);
long __syscall54(long fd, long request, ...);
long __syscall57(long pid, long gpid);
long __syscall60(long mask);
long __syscall63(long oldfd, long newfd);
long __syscall64();
long __syscall65();
long __syscall66();
long __syscall75(long resource, long limit);
long __syscall77(long who, long usage);
long __syscall83(long target, long linkpath);
long __syscall85(long path, long buf, long bufsize);
long __syscall91(long addr, long len);
long __syscall94(long fd, long mode);
long __syscall96(long which, long who);
long __syscall97(long which, long who, long prio);
long __syscall102(long call, long args);
long __syscall104(long which, long new_value, long old_value);
long __syscall114(long pid, long wstatus, long options, long rusage);
long __syscall121(long name, long size);
long __syscall122(long buf);
long __syscall125(long addr, long len, long size);
long __syscall132(long pid);
long __syscall133(long fd);
long __syscall142(long nfds, long readfds, long writefds, long exceptfds, long timeout);
long __syscall144(long addr, long len, long flags);
long __syscall147(long pid);
long __syscall148(long fd);
long __syscall150(long addr, long len);
long __syscall151(long addr, long len);
long __syscall152(long flags);
long __syscall153();
long __syscall163(long old_addr, long old_size, long new_size, long flags, long new_addr);
long __syscall168(long fds, long nfds, long timeout);
long __syscall178(long tgid, long sig, long uinfo);
long __syscall180(long fd, long buf, long count, long zero, long low, long high);
long __syscall181(long fd, long buf, long count, long zero, long low, long high);
long __syscall183(long buf, long size);
long __syscall191(long resource, long rlim);
long __syscall192(long addr, long len, long prot, long flags, long fd, long off);
long __syscall193(long path, long zero, long low, long high);
long __syscall194(long fd, long zero, long low, long high);
long __syscall195(long path, long buf);
long __syscall196(long path, long buf);
long __syscall197(long fd, long buf);
long __syscall198(long path, long owner, long group);
long __syscall199();
long __syscall200();
long __syscall201();
long __syscall202();
long __syscall203(long ruid, long euid);
long __syscall204(long rgid, long egid);
long __syscall205(long size, long list);
long __syscall207(long fd, long owner, long group);
long __syscall208(long ruid, long euid, long suid);
long __syscall209(long ruid, long euid, long suid);
long __syscall210(long rgid, long egid, long sgid);
long __syscall211(long rgid, long egid, long sgid);
long __syscall212(long path, long owner, long group);
long __syscall213(long uid);
long __syscall214(long uid);
long __syscall218(long addr, long length, long vec);
long __syscall219(long addr, long length, long advice);
long __syscall220(long fd, long dirp, long count);
long __syscall221(long fd, long cmd, ...);
long __syscall252(long status);
long __syscall268(long path, long size, long buf);
long __syscall269(long fd, long size, long buf);
long __syscall272(long fd, long zero, long low, long high, long low2, long high2, long advice);
long __syscall295(long dirfd, long path, long flags, ...);
long __syscall296(long dirfd, long path, long mode);
long __syscall297(long dirfd, long path, long mode, long dev);
long __syscall298(long dirfd, long path, long owner, long group, long flags);
long __syscall299(long dirfd, long path, long times);
long __syscall300(long dirfd, long path, long buf, long flags);
long __syscall301(long dirfd, long path, long flags);
long __syscall302(long olddirfd, long oldpath, long newdirfd, long newpath);
long __syscall303(long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
long __syscall304(long target, long newdirfd, long linkpath);
long __syscall305(long dirfd, long path, long bug, long bufsize);
long __syscall306(long dirfd, long path, long mode, ...);
long __syscall307(long dirfd, long path, long amode, long flags);
long __syscall308(long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
long __syscall320(long dirfd, long path, long times, long flags);
long __syscall324(long fd, long mode, long off_low, long off_high, long len_low, long len_high);
long __syscall330(long fd, long suggestfd, long flags);
long __syscall331(long fds, long flags);
long __syscall333(long fd, long iov, long iovcnt, long low, long high);
long __syscall334(long fd, long iov, long iovcnt, long low, long high);
long __syscall337(long sockfd, long msgvec, long vlen, long flags, ...);
long __syscall340(long pid, long resource, long new_limit, long old_limit);
long __syscall345(long sockfd, long msgvec, long vlen, long flags, ...);

#ifdef __cplusplus
}
#endif
