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
#endif
#include "futex.h"

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
	uintptr_t canary, canary2;
	pid_t tid, pid;
	int tsd_used, errno_val;
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
		volatile void *volatile head;
		long off;
		volatile void *volatile pending;
	} robust_list;
	int unblock_cancel;
	volatile int timer_id;
	locale_t locale;
	volatile int killlock[2];
	volatile int exitlock[2];
	volatile int startlock[2];
	unsigned long sigmask[_NSIG/8/sizeof(long)];
	char *dlerror_buf;
	int dlerror_flag;
	void *stdio_locks;
	uintptr_t canary_at_end;
	void **dtv_copy;
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
#define _m_lock __u.__vi[1]
#define _m_waiters __u.__vi[2]
#define _m_prev __u.__p[3]
#define _m_next __u.__p[4]
#define _m_count __u.__i[5]
#define _c_shared __u.__p[0]
#define _c_seq __u.__vi[2]
#define _c_waiters __u.__vi[3]
#define _c_clock __u.__i[4]
#define _c_lock __u.__vi[8]
#define _c_head __u.__p[1]
#define _c_tail __u.__p[5]
#define _rw_lock __u.__vi[0]
#define _rw_waiters __u.__vi[1]
#define _rw_shared __u.__i[2]
#ifdef __EMSCRIPTEN__
// XXX Emscripten: The spec allows detecting when multiple write locks would deadlock, so use an extra field
// _rw_wr_owner to record which thread owns the write lock in order to avoid hangs.
// Points to the pthread that currently has the write lock.
#define _rw_wr_owner __u.__vi[3]
#endif
#define _b_lock __u.__vi[0]
#define _b_waiters __u.__vi[1]
#define _b_limit __u.__i[2]
#define _b_count __u.__vi[3]
#define _b_waiters2 __u.__vi[4]
#define _b_inst __u.__p[3]

#include "pthread_arch.h"

#ifndef CANARY
#define CANARY canary
#endif

#ifndef DTP_OFFSET
#define DTP_OFFSET 0
#endif

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

void __vm_wait(void);
void __vm_lock(void);
void __vm_unlock(void);

int __timedwait(volatile int *, int, clockid_t, const struct timespec *, int);
int __timedwait_cp(volatile int *, int, clockid_t, const struct timespec *, int);
void __wait(volatile int *, volatile int *, int, int);
static inline void __wake(volatile void *addr, int cnt, int priv)
{
	if (priv) priv = 128;
	if (cnt<0) cnt = INT_MAX;
#ifdef __EMSCRIPTEN__
	emscripten_futex_wake(addr, (cnt)<0?INT_MAX:(cnt));
#else
	__syscall(SYS_futex, addr, FUTEX_WAKE|priv, cnt) != -ENOSYS ||
	__syscall(SYS_futex, addr, FUTEX_WAKE, cnt);
#endif
}

void __acquire_ptc(void);
void __release_ptc(void);
void __inhibit_ptc(void);

void __block_all_sigs(void *);
void __block_app_sigs(void *);
void __restore_sigs(void *);

#define DEFAULT_STACK_SIZE 81920
#define DEFAULT_GUARD_SIZE PAGE_SIZE

#define __ATTRP_C11_THREAD ((void*)(uintptr_t)-1)

#ifdef __EMSCRIPTEN__
void __emscripten_init_pthread(pthread_t thread);
#if !__EMSCRIPTEN_PTHREADS__
pthread_t __emscripten_pthread_stub(void);
#endif
#endif
#endif
