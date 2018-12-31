#ifndef _PTHREAD_IMPL_H
#define _PTHREAD_IMPL_H

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include "libc.h"
#include "syscall.h"
#include "atomic.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/threading.h>
#else
#include "futex.h"
#endif

#define pthread __pthread

struct pthread {
// XXX Emscripten: Need some custom thread control structures.
#ifdef __EMSCRIPTEN__
	// Note: The specific order of these fields is important, since these are accessed
	// by direct pointer arithmetic in pthread-main.js.
	int threadStatus; // 0: thread not exited, 1: exited.
	int threadExitCode; // Thread exit code.
	int tempDoublePtr[3]; // Temporary memory area for double operations in runtime.
	void *profilerBlock; // If --threadprofiling is enabled, this pointer is allocated to contain internal information about the thread state for profiling purposes.
#endif

	struct pthread *self;
	void **dtv, *unused1, *unused2;
	uintptr_t sysinfo;
	uintptr_t canary;
	pid_t tid, pid;
	int tsd_used, errno_val, *errno_ptr;
	volatile int cancel, canceldisable, cancelasync;
	int detached;
	unsigned char *map_base;
	size_t map_size;
	void *stack;
	size_t stack_size;
	void *start_arg;
	void *(*start)(void *);
	void *result;
	struct __ptcb *cancelbuf;
	void **tsd;
	pthread_attr_t attr;
	volatile int dead;
	struct {
		void **head;
		long off;
		void *pending;
	} robust_list;
	int unblock_cancel;
	int timer_id;
	locale_t locale;
	int killlock[2];
	int exitlock[2];
	int startlock[2];
	unsigned long sigmask[_NSIG/8/sizeof(long)];
};

struct __timer {
	int timerid;
	pthread_t thread;
};

#define __SU (sizeof(size_t)/sizeof(int))

#define _a_stacksize __u.__s[0]
#define _a_guardsize __u.__s[1]
#define _a_stackaddr __u.__s[2]
#define _a_detach __u.__i[3*__SU+0]
#define _a_sched __u.__i[3*__SU+1]
#define _a_policy __u.__i[3*__SU+2]
#define _a_prio __u.__i[3*__SU+3]
#define _m_type __u.__i[0]
#define _m_lock __u.__i[1]
#define _m_waiters __u.__i[2]
#define _m_prev __u.__p[3]
#define _m_next __u.__p[4]
#define _m_count __u.__i[5]
#ifdef __EMSCRIPTEN__
#define _m_addr __u.__i[6]
#endif
#define _c_mutex __u.__p[0]
#define _c_seq __u.__i[2]
#define _c_waiters __u.__i[3]
#define _c_clock __u.__i[4]
#define _c_lock __u.__i[5]
#define _c_lockwait __u.__i[6]
#define _c_waiters2 __u.__i[7]
#define _c_destroy __u.__i[8]
#define _rw_lock __u.__i[0]
#define _rw_waiters __u.__i[1]
#ifdef __EMSCRIPTEN__
// XXX Emscripten: The spec allows detecting when multiple write locks would deadlock, so use an extra field
// _rw_wr_owner to record which thread owns the write lock in order to avoid hangs.
// Points to the pthread that currently has the write lock.
#define _rw_wr_owner __u.__i[2]
#endif
#define _b_lock __u.__i[0]
#define _b_waiters __u.__i[1]
#define _b_limit __u.__i[2]
#define _b_count __u.__i[3]
#define _b_waiters2 __u.__i[4]
#define _b_inst __u.__p[3]

#include "pthread_arch.h"

#define SIGTIMER 32
#define SIGCANCEL 33
#define SIGSYNCCALL 34

#define SIGALL_SET ((sigset_t *)(const unsigned long long [2]){ -1,-1 })
#define SIGPT_SET \
	((sigset_t *)(const unsigned long [_NSIG/8/sizeof(long)]){ \
	[sizeof(long)==4] = 3UL<<(32*(sizeof(long)>4)) })
#define SIGTIMER_SET \
	((sigset_t *)(const unsigned long [_NSIG/8/sizeof(long)]){ \
	 0x80000000 })

pthread_t __pthread_self_init(void);

int __clone(int (*)(void *), void *, int, void *, ...);
int __set_thread_area(void *);
int __libc_sigaction(int, const struct sigaction *, struct sigaction *);
int __libc_sigprocmask(int, const sigset_t *, sigset_t *);
void __lock(volatile int *);
void __unmapself(void *, size_t);

int __timedwait(volatile int *, int, clockid_t, const struct timespec *, void (*)(void *), void *, int);
void __wait(volatile int *, volatile int *, int, int);

#ifdef __EMSCRIPTEN__
#define __wake(addr, cnt, priv) emscripten_futex_wake((void*)addr, (cnt)<0?INT_MAX:(cnt))
#else
#define __wake(addr, cnt, priv) \
	__syscall(SYS_futex, addr, FUTEX_WAKE, (cnt)<0?INT_MAX:(cnt))
#endif

void __acquire_ptc();
void __release_ptc();
void __inhibit_ptc();

void __block_all_sigs(void *);
void __block_app_sigs(void *);
void __restore_sigs(void *);

#define DEFAULT_STACK_SIZE 81920
#define DEFAULT_GUARD_SIZE PAGE_SIZE

#endif
