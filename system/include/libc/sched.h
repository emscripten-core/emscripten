#ifndef _SCHED_H
#define _SCHED_H
#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#define __NEED_struct_timespec
#define __NEED_pid_t
#define __NEED_time_t

#include <bits/alltypes.h>

struct sched_param {
	int sched_priority;
	int sched_ss_low_priority;
	struct timespec sched_ss_repl_period;
	struct timespec sched_ss_init_budget;
	int sched_ss_max_repl;
};

int    sched_get_priority_max(int);
int    sched_get_priority_min(int);
int    sched_getparam(pid_t, struct sched_param *);
int    sched_getscheduler(pid_t);
int    sched_rr_get_interval(pid_t, struct timespec *);
int    sched_setparam(pid_t, const struct sched_param *);
int    sched_setscheduler(pid_t, int, const struct sched_param *);
int     sched_yield(void);

#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_BATCH 3
#define SCHED_IDLE 5
#define SCHED_RESET_ON_FORK 0x40000000

#ifdef _GNU_SOURCE
#define CSIGNAL		0x000000ff
#define CLONE_VM	0x00000100
#define CLONE_FS	0x00000200
#define CLONE_FILES	0x00000400
#define CLONE_SIGHAND	0x00000800
#define CLONE_PTRACE	0x00002000
#define CLONE_VFORK	0x00004000
#define CLONE_PARENT	0x00008000
#define CLONE_THREAD	0x00010000
#define CLONE_NEWNS	0x00020000
#define CLONE_SYSVSEM	0x00040000
#define CLONE_SETTLS	0x00080000
#define CLONE_PARENT_SETTID	0x00100000
#define CLONE_CHILD_CLEARTID	0x00200000
#define CLONE_DETACHED	0x00400000
#define CLONE_UNTRACED	0x00800000
#define CLONE_CHILD_SETTID	0x01000000
#define CLONE_NEWUTS	0x04000000
#define CLONE_NEWIPC	0x08000000
#define CLONE_NEWUSER	0x10000000
#define CLONE_NEWPID	0x20000000
#define CLONE_NEWNET	0x40000000
#define CLONE_IO	0x80000000
int clone (int (*)(void *), void *, int, void *, ...);
int unshare(int);
int setns(int, int);
#endif

#ifdef __cplusplus
}
#endif
#endif
