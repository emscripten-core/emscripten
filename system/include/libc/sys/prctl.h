#ifndef _SYS_PRCTL_H
#define _SYS_PRCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define PR_SET_PDEATHSIG  1
#define PR_GET_PDEATHSIG  2
#define PR_GET_DUMPABLE   3
#define PR_SET_DUMPABLE   4
#define PR_GET_UNALIGN   5
#define PR_SET_UNALIGN   6
#define PR_UNALIGN_NOPRINT 1
#define PR_UNALIGN_SIGBUS 2
#define PR_GET_KEEPCAPS   7
#define PR_SET_KEEPCAPS   8
#define PR_GET_FPEMU  9
#define PR_SET_FPEMU 10
#define PR_FPEMU_NOPRINT 1
#define PR_FPEMU_SIGFPE 2
#define PR_GET_FPEXC 11
#define PR_SET_FPEXC 12
#define PR_FP_EXC_SW_ENABLE 0x80
#define PR_FP_EXC_DIV  0x010000
#define PR_FP_EXC_OVF  0x020000
#define PR_FP_EXC_UND  0x040000
#define PR_FP_EXC_RES  0x080000
#define PR_FP_EXC_INV  0x100000
#define PR_FP_EXC_DISABLED 0
#define PR_FP_EXC_NONRECOV 1
#define PR_FP_EXC_ASYNC 2
#define PR_FP_EXC_PRECISE 3
#define PR_GET_TIMING   13
#define PR_SET_TIMING   14
#define PR_TIMING_STATISTICAL  0
#define PR_TIMING_TIMESTAMP    1
#define PR_SET_NAME    15
#define PR_GET_NAME    16
#define PR_GET_ENDIAN 19
#define PR_SET_ENDIAN 20
#define PR_ENDIAN_BIG 0
#define PR_ENDIAN_LITTLE 1
#define PR_ENDIAN_PPC_LITTLE 2
#define PR_GET_SECCOMP 21
#define PR_SET_SECCOMP 22
#define PR_CAPBSET_READ 23
#define PR_CAPBSET_DROP 24
#define PR_GET_TSC 25
#define PR_SET_TSC 26
#define PR_TSC_ENABLE 1
#define PR_TSC_SIGSEGV 2
#define PR_GET_SECUREBITS 27
#define PR_SET_SECUREBITS 28
#define PR_SET_TIMERSLACK 29
#define PR_GET_TIMERSLACK 30

#define PR_TASK_PERF_EVENTS_DISABLE             31
#define PR_TASK_PERF_EVENTS_ENABLE              32

#define PR_MCE_KILL     33
#define PR_MCE_KILL_CLEAR   0
#define PR_MCE_KILL_SET     1
#define PR_MCE_KILL_LATE    0
#define PR_MCE_KILL_EARLY   1
#define PR_MCE_KILL_DEFAULT 2
#define PR_MCE_KILL_GET 34

#define PR_SET_MM               35
#define PR_SET_MM_START_CODE           1
#define PR_SET_MM_END_CODE             2
#define PR_SET_MM_START_DATA           3
#define PR_SET_MM_END_DATA             4
#define PR_SET_MM_START_STACK          5
#define PR_SET_MM_START_BRK            6
#define PR_SET_MM_BRK                  7
#define PR_SET_MM_ARG_START            8
#define PR_SET_MM_ARG_END              9
#define PR_SET_MM_ENV_START            10
#define PR_SET_MM_ENV_END              11
#define PR_SET_MM_AUXV                 12
#define PR_SET_MM_EXE_FILE             13

#define PR_SET_PTRACER 0x59616d61
#define PR_SET_PTRACER_ANY (-1UL)

#define PR_SET_CHILD_SUBREAPER  36
#define PR_GET_CHILD_SUBREAPER  37

#define PR_SET_NO_NEW_PRIVS     38
#define PR_GET_NO_NEW_PRIVS     39

#define PR_GET_TID_ADDRESS      40

int prctl (int, ...);

#ifdef __cplusplus
}
#endif

#endif
