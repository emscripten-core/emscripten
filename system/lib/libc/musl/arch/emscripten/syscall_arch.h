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

#define DECLARE_SYSCALL(NAME) EM_IMPORT(NAME) long NAME

DECLARE_SYSCALL(__syscall1)(long exit_code);
DECLARE_SYSCALL(__syscall3)(long fd, long buf, long count);
DECLARE_SYSCALL(__syscall4)(long fd, long buf, long count);
DECLARE_SYSCALL(__syscall5)(long path, long flags, ...);  // mode is optional
DECLARE_SYSCALL(__syscall9)(long oldpath, long newpath);
DECLARE_SYSCALL(__syscall10)(long path);
DECLARE_SYSCALL(__syscall12)(long path);
DECLARE_SYSCALL(__syscall14)(long path, long mode, long dev);
DECLARE_SYSCALL(__syscall15)(long path, long mode);
DECLARE_SYSCALL(__syscall20)(void);
DECLARE_SYSCALL(__syscall29)(void);
DECLARE_SYSCALL(__syscall33)(long path, long amode);
DECLARE_SYSCALL(__syscall34)(long inc);
DECLARE_SYSCALL(__syscall36)(void);
DECLARE_SYSCALL(__syscall38)(long old_path, long new_path);
DECLARE_SYSCALL(__syscall39)(long path, long mode);
DECLARE_SYSCALL(__syscall40)(long path);
DECLARE_SYSCALL(__syscall41)(long fd);
DECLARE_SYSCALL(__syscall42)(long fd);
DECLARE_SYSCALL(__syscall51)(long filename);
DECLARE_SYSCALL(__syscall54)(long fd, long request, ...);
DECLARE_SYSCALL(__syscall57)(long pid, long gpid);
DECLARE_SYSCALL(__syscall60)(long mask);
DECLARE_SYSCALL(__syscall63)(long oldfd, long newfd);
DECLARE_SYSCALL(__syscall64)(void);
DECLARE_SYSCALL(__syscall65)(void);
DECLARE_SYSCALL(__syscall66)(void);
DECLARE_SYSCALL(__syscall75)(long resource, long limit);
DECLARE_SYSCALL(__syscall77)(long who, long usage);
DECLARE_SYSCALL(__syscall83)(long target, long linkpath);
DECLARE_SYSCALL(__syscall85)(long path, long buf, long bufsize);
DECLARE_SYSCALL(__syscall91)(long addr, long len);
DECLARE_SYSCALL(__syscall94)(long fd, long mode);
DECLARE_SYSCALL(__syscall96)(long which, long who);
DECLARE_SYSCALL(__syscall97)(long which, long who, long prio);
DECLARE_SYSCALL(__syscall102)(long call, long args);
DECLARE_SYSCALL(__syscall104)(long which, long new_value, long old_value);
DECLARE_SYSCALL(__syscall114)(long pid, long wstatus, long options, long rusage);
DECLARE_SYSCALL(__syscall121)(long name, long size);
DECLARE_SYSCALL(__syscall122)(long buf);
DECLARE_SYSCALL(__syscall125)(long addr, long len, long size);
DECLARE_SYSCALL(__syscall132)(long pid);
DECLARE_SYSCALL(__syscall133)(long fd);
DECLARE_SYSCALL(__syscall142)(long nfds, long readfds, long writefds, long exceptfds, long timeout);
DECLARE_SYSCALL(__syscall144)(long addr, long len, long flags);
DECLARE_SYSCALL(__syscall147)(long pid);
DECLARE_SYSCALL(__syscall148)(long fd);
DECLARE_SYSCALL(__syscall150)(long addr, long len);
DECLARE_SYSCALL(__syscall151)(long addr, long len);
DECLARE_SYSCALL(__syscall152)(long flags);
DECLARE_SYSCALL(__syscall153)(void);
DECLARE_SYSCALL(__syscall163)(long old_addr, long old_size, long new_size, long flags, long new_addr);
DECLARE_SYSCALL(__syscall168)(long fds, long nfds, long timeout);
DECLARE_SYSCALL(__syscall178)(long tgid, long sig, long uinfo);
DECLARE_SYSCALL(__syscall180)(long fd, long buf, long count, long zero, long low, long high);
DECLARE_SYSCALL(__syscall181)(long fd, long buf, long count, long zero, long low, long high);
DECLARE_SYSCALL(__syscall183)(long buf, long size);
DECLARE_SYSCALL(__syscall191)(long resource, long rlim);
DECLARE_SYSCALL(__syscall192)(long addr, long len, long prot, long flags, long fd, long off);
DECLARE_SYSCALL(__syscall193)(long path, long zero, long low, long high);
DECLARE_SYSCALL(__syscall194)(long fd, long zero, long low, long high);
DECLARE_SYSCALL(__syscall195)(long path, long buf);
DECLARE_SYSCALL(__syscall196)(long path, long buf);
DECLARE_SYSCALL(__syscall197)(long fd, long buf);
DECLARE_SYSCALL(__syscall198)(long path, long owner, long group);
DECLARE_SYSCALL(__syscall199)(void);
DECLARE_SYSCALL(__syscall200)(void);
DECLARE_SYSCALL(__syscall201)(void);
DECLARE_SYSCALL(__syscall202)(void);
DECLARE_SYSCALL(__syscall203)(long ruid, long euid);
DECLARE_SYSCALL(__syscall204)(long rgid, long egid);
DECLARE_SYSCALL(__syscall205)(long size, long list);
DECLARE_SYSCALL(__syscall207)(long fd, long owner, long group);
DECLARE_SYSCALL(__syscall208)(long ruid, long euid, long suid);
DECLARE_SYSCALL(__syscall209)(long ruid, long euid, long suid);
DECLARE_SYSCALL(__syscall210)(long rgid, long egid, long sgid);
DECLARE_SYSCALL(__syscall211)(long rgid, long egid, long sgid);
DECLARE_SYSCALL(__syscall212)(long path, long owner, long group);
DECLARE_SYSCALL(__syscall213)(long uid);
DECLARE_SYSCALL(__syscall214)(long uid);
DECLARE_SYSCALL(__syscall218)(long addr, long length, long vec);
DECLARE_SYSCALL(__syscall219)(long addr, long length, long advice);
DECLARE_SYSCALL(__syscall220)(long fd, long dirp, long count);
DECLARE_SYSCALL(__syscall221)(long fd, long cmd, ...);
DECLARE_SYSCALL(__syscall252)(long status);
DECLARE_SYSCALL(__syscall268)(long path, long size, long buf);
DECLARE_SYSCALL(__syscall269)(long fd, long size, long buf);
DECLARE_SYSCALL(__syscall272)(long fd, long zero, long low, long high, long low2, long high2, long advice);
DECLARE_SYSCALL(__syscall295)(long dirfd, long path, long flags, ...);
DECLARE_SYSCALL(__syscall296)(long dirfd, long path, long mode);
DECLARE_SYSCALL(__syscall297)(long dirfd, long path, long mode, long dev);
DECLARE_SYSCALL(__syscall298)(long dirfd, long path, long owner, long group, long flags);
DECLARE_SYSCALL(__syscall299)(long dirfd, long path, long times);
DECLARE_SYSCALL(__syscall300)(long dirfd, long path, long buf, long flags);
DECLARE_SYSCALL(__syscall301)(long dirfd, long path, long flags);
DECLARE_SYSCALL(__syscall302)(long olddirfd, long oldpath, long newdirfd, long newpath);
DECLARE_SYSCALL(__syscall303)(long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
DECLARE_SYSCALL(__syscall304)(long target, long newdirfd, long linkpath);
DECLARE_SYSCALL(__syscall305)(long dirfd, long path, long bug, long bufsize);
DECLARE_SYSCALL(__syscall306)(long dirfd, long path, long mode, ...);
DECLARE_SYSCALL(__syscall307)(long dirfd, long path, long amode, long flags);
DECLARE_SYSCALL(__syscall308)(long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
DECLARE_SYSCALL(__syscall320)(long dirfd, long path, long times, long flags);
DECLARE_SYSCALL(__syscall324)(long fd, long mode, long off_low, long off_high, long len_low, long len_high);
DECLARE_SYSCALL(__syscall330)(long fd, long suggestfd, long flags);
DECLARE_SYSCALL(__syscall331)(long fds, long flags);
DECLARE_SYSCALL(__syscall333)(long fd, long iov, long iovcnt, long low, long high);
DECLARE_SYSCALL(__syscall334)(long fd, long iov, long iovcnt, long low, long high);
DECLARE_SYSCALL(__syscall337)(long sockfd, long msgvec, long vlen, long flags, ...);
DECLARE_SYSCALL(__syscall340)(long pid, long resource, long new_limit, long old_limit);
DECLARE_SYSCALL(__syscall345)(long sockfd, long msgvec, long vlen, long flags, ...);

#ifdef __cplusplus
}
#endif
