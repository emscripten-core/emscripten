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

long __syscall1(int which, ...);
long __syscall3(int which, ...);
long __syscall4(int which, ...);
long __syscall5(int which, ...);
long __syscall6(int which, ...);
long __syscall9(int which, ...);
long __syscall10(int which, ...);
long __syscall12(int which, ...);
long __syscall14(int which, ...);
long __syscall15(int which, ...);
long __syscall20(int which, ...);
long __syscall29(int which, ...);
long __syscall33(int which, ...);
long __syscall34(int which, ...);
long __syscall36(int which, ...);
long __syscall38(int which, ...);
long __syscall39(int which, ...);
long __syscall40(int which, ...);
long __syscall41(int which, ...);
long __syscall42(int which, ...);
long __syscall51(int which, ...);
long __syscall54(int which, ...);
long __syscall57(int which, ...);
long __syscall60(int which, ...);
long __syscall63(int which, ...);
long __syscall64(int which, ...);
long __syscall65(int which, ...);
long __syscall66(int which, ...);
long __syscall75(int which, ...);
long __syscall77(int which, ...);
long __syscall83(int which, ...);
long __syscall85(int which, ...);
long __syscall91(int which, ...);
long __syscall94(int which, ...);
long __syscall96(int which, ...);
long __syscall97(int which, ...);
long __syscall102(int which, ...);
long __syscall104(int which, ...);
long __syscall114(int which, ...);
long __syscall118(int which, ...);
long __syscall121(int which, ...);
long __syscall122(int which, ...);
long __syscall125(int which, ...);
long __syscall132(int which, ...);
long __syscall133(int which, ...);
long __syscall140(int which, ...);
long __syscall142(int which, ...);
long __syscall144(int which, ...);
long __syscall145(int which, ...);
long __syscall146(int which, ...);
long __syscall147(int which, ...);
long __syscall148(int which, ...);
long __syscall150(int which, ...);
long __syscall151(int which, ...);
long __syscall152(int which, ...);
long __syscall153(int which, ...);
long __syscall163(int which, ...);
long __syscall168(int which, ...);
long __syscall178(int which, ...);
long __syscall180(int which, ...);
long __syscall181(int which, ...);
long __syscall183(int which, ...);
long __syscall191(int which, ...);
long __syscall192(int which, ...);
long __syscall193(int which, ...);
long __syscall194(int which, ...);
long __syscall195(int which, ...);
long __syscall196(int which, ...);
long __syscall197(int which, ...);
long __syscall198(int which, ...);
long __syscall199(int which, ...);
long __syscall200(int which, ...);
long __syscall201(int which, ...);
long __syscall202(int which, ...);
long __syscall203(int which, ...);
long __syscall204(int which, ...);
long __syscall205(int which, ...);
long __syscall207(int which, ...);
long __syscall208(int which, ...);
long __syscall209(int which, ...);
long __syscall211(int which, ...);
long __syscall212(int which, ...);
long __syscall218(int which, ...);
long __syscall219(int which, ...);
long __syscall220(int which, ...);
long __syscall221(int which, ...);
long __syscall252(int which, ...);
long __syscall265(int which, ...);
long __syscall268(int which, ...);
long __syscall269(int which, ...);
long __syscall272(int which, ...);
long __syscall295(int which, ...);
long __syscall296(int which, ...);
long __syscall297(int which, ...);
long __syscall298(int which, ...);
long __syscall299(int which, ...);
long __syscall300(int which, ...);
long __syscall301(int which, ...);
long __syscall302(int which, ...);
long __syscall303(int which, ...);
long __syscall304(int which, ...);
long __syscall305(int which, ...);
long __syscall306(int which, ...);
long __syscall307(int which, ...);
long __syscall308(int which, ...);
long __syscall320(int which, ...);
long __syscall324(int which, ...);
long __syscall330(int which, ...);
long __syscall331(int which, ...);
long __syscall333(int which, ...);
long __syscall334(int which, ...);
long __syscall337(int which, ...);
long __syscall340(int which, ...);
long __syscall345(int which, ...);

#undef SYS_futimesat

#ifdef __cplusplus
}
#endif
