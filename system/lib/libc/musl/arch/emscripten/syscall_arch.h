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

#define EM_SYSCALL_ARGS int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7
long __syscall1(EM_SYSCALL_ARGS);
long __syscall3(EM_SYSCALL_ARGS);
long __syscall4(EM_SYSCALL_ARGS);
long __syscall5(EM_SYSCALL_ARGS);
long __syscall6(EM_SYSCALL_ARGS);
long __syscall9(EM_SYSCALL_ARGS);
long __syscall10(EM_SYSCALL_ARGS);
long __syscall12(EM_SYSCALL_ARGS);
long __syscall14(EM_SYSCALL_ARGS);
long __syscall15(EM_SYSCALL_ARGS);
long __syscall20(EM_SYSCALL_ARGS);
long __syscall29(EM_SYSCALL_ARGS);
long __syscall33(EM_SYSCALL_ARGS);
long __syscall34(EM_SYSCALL_ARGS);
long __syscall36(EM_SYSCALL_ARGS);
long __syscall38(EM_SYSCALL_ARGS);
long __syscall39(EM_SYSCALL_ARGS);
long __syscall40(EM_SYSCALL_ARGS);
long __syscall41(EM_SYSCALL_ARGS);
long __syscall42(EM_SYSCALL_ARGS);
long __syscall51(EM_SYSCALL_ARGS);
long __syscall54(EM_SYSCALL_ARGS);
long __syscall57(EM_SYSCALL_ARGS);
long __syscall60(EM_SYSCALL_ARGS);
long __syscall63(EM_SYSCALL_ARGS);
long __syscall64(EM_SYSCALL_ARGS);
long __syscall65(EM_SYSCALL_ARGS);
long __syscall66(EM_SYSCALL_ARGS);
long __syscall75(EM_SYSCALL_ARGS);
long __syscall77(EM_SYSCALL_ARGS);
long __syscall83(EM_SYSCALL_ARGS);
long __syscall85(EM_SYSCALL_ARGS);
long __syscall91(EM_SYSCALL_ARGS);
long __syscall94(EM_SYSCALL_ARGS);
long __syscall96(EM_SYSCALL_ARGS);
long __syscall97(EM_SYSCALL_ARGS);
long __syscall102(EM_SYSCALL_ARGS);
long __syscall104(EM_SYSCALL_ARGS);
long __syscall114(EM_SYSCALL_ARGS);
long __syscall118(EM_SYSCALL_ARGS);
long __syscall121(EM_SYSCALL_ARGS);
long __syscall122(EM_SYSCALL_ARGS);
long __syscall125(EM_SYSCALL_ARGS);
long __syscall132(EM_SYSCALL_ARGS);
long __syscall133(EM_SYSCALL_ARGS);
long __syscall140(EM_SYSCALL_ARGS);
long __syscall142(EM_SYSCALL_ARGS);
long __syscall144(EM_SYSCALL_ARGS);
long __syscall145(EM_SYSCALL_ARGS);
long __syscall146(EM_SYSCALL_ARGS);
long __syscall147(EM_SYSCALL_ARGS);
long __syscall148(EM_SYSCALL_ARGS);
long __syscall150(EM_SYSCALL_ARGS);
long __syscall151(EM_SYSCALL_ARGS);
long __syscall152(EM_SYSCALL_ARGS);
long __syscall153(EM_SYSCALL_ARGS);
long __syscall163(EM_SYSCALL_ARGS);
long __syscall168(EM_SYSCALL_ARGS);
long __syscall178(EM_SYSCALL_ARGS);
long __syscall180(EM_SYSCALL_ARGS);
long __syscall181(EM_SYSCALL_ARGS);
long __syscall183(EM_SYSCALL_ARGS);
long __syscall191(EM_SYSCALL_ARGS);
long __syscall192(EM_SYSCALL_ARGS);
long __syscall193(EM_SYSCALL_ARGS);
long __syscall194(EM_SYSCALL_ARGS);
long __syscall195(EM_SYSCALL_ARGS);
long __syscall196(EM_SYSCALL_ARGS);
long __syscall197(EM_SYSCALL_ARGS);
long __syscall198(EM_SYSCALL_ARGS);
long __syscall199(EM_SYSCALL_ARGS);
long __syscall200(EM_SYSCALL_ARGS);
long __syscall201(EM_SYSCALL_ARGS);
long __syscall202(EM_SYSCALL_ARGS);
long __syscall203(EM_SYSCALL_ARGS);
long __syscall204(EM_SYSCALL_ARGS);
long __syscall205(EM_SYSCALL_ARGS);
long __syscall207(EM_SYSCALL_ARGS);
long __syscall208(EM_SYSCALL_ARGS);
long __syscall209(EM_SYSCALL_ARGS);
long __syscall211(EM_SYSCALL_ARGS);
long __syscall212(EM_SYSCALL_ARGS);
long __syscall218(EM_SYSCALL_ARGS);
long __syscall219(EM_SYSCALL_ARGS);
long __syscall220(EM_SYSCALL_ARGS);
long __syscall221(EM_SYSCALL_ARGS);
long __syscall252(EM_SYSCALL_ARGS);
long __syscall265(EM_SYSCALL_ARGS);
long __syscall268(EM_SYSCALL_ARGS);
long __syscall269(EM_SYSCALL_ARGS);
long __syscall272(EM_SYSCALL_ARGS);
long __syscall295(EM_SYSCALL_ARGS);
long __syscall296(EM_SYSCALL_ARGS);
long __syscall297(EM_SYSCALL_ARGS);
long __syscall298(EM_SYSCALL_ARGS);
long __syscall299(EM_SYSCALL_ARGS);
long __syscall300(EM_SYSCALL_ARGS);
long __syscall301(EM_SYSCALL_ARGS);
long __syscall302(EM_SYSCALL_ARGS);
long __syscall303(EM_SYSCALL_ARGS);
long __syscall304(EM_SYSCALL_ARGS);
long __syscall305(EM_SYSCALL_ARGS);
long __syscall306(EM_SYSCALL_ARGS);
long __syscall307(EM_SYSCALL_ARGS);
long __syscall308(EM_SYSCALL_ARGS);
long __syscall320(EM_SYSCALL_ARGS);
long __syscall324(EM_SYSCALL_ARGS);
long __syscall330(EM_SYSCALL_ARGS);
long __syscall331(EM_SYSCALL_ARGS);
long __syscall333(EM_SYSCALL_ARGS);
long __syscall334(EM_SYSCALL_ARGS);
long __syscall337(EM_SYSCALL_ARGS);
long __syscall340(EM_SYSCALL_ARGS);
long __syscall345(EM_SYSCALL_ARGS);

#undef SYS_futimesat

#ifdef __cplusplus
}
#endif
