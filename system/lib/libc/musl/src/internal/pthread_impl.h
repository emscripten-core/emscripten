#ifndef _PTHREAD_IMPL_H
#define _PTHREAD_IMPL_H

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include "libc.h"
#include "syscall.h"
#include "atomic.h"
#ifdef __EMSCRIPTEN__
#include "em_task_queue.h"
#include "thread_mailbox.h"
#include "threading_internal.h"
#include <emscripten/threading.h>
#endif
#include "futex.h"

#include "pthread_arch.h"

#define pthread __pthread

struct pthread {
	/* Part 1 -- these fields may be external or
	 * internal (accessed via asm) ABI. Do not change. */
	struct pthread *self;
#ifndef TLS_ABOVE_TP
	uintptr_t *dtv;
#endif
	struct pthread *prev, *next; /* non-ABI */
	uintptr_t sysinfo;
#ifndef TLS_ABOVE_TP
#ifdef CANARY_PAD
	uintptr_t canary_pad;
#endif
	uintptr_t canary;
#endif

	/* Part 2 -- implementation details, non-ABI. */
	int tid;
	int errno_val;
	volatile int detach_state;
	volatile int cancel;
	volatile unsigned char canceldisable, cancelasync;
	unsigned char tsd_used:1;
	unsigned char dlerror_flag:1;
	unsigned char *map_base;
	size_t map_size;
	void *stack;
	size_t stack_size;
	size_t guard_size;
	void *result;
	struct __ptcb *cancelbuf;
	void **tsd;
	struct {
		volatile void *volatile head;
		long off;
		volatile void *volatile pending;
	} robust_list;
	int h_errno_val;
	volatile int timer_id;
	locale_t locale;
	volatile int killlock[1];
	char *dlerror_buf;
	void *stdio_locks;

	/* Part 3 -- the positions of these fields relative to
	 * the end of the structure is external and internal ABI. */
#ifdef TLS_ABOVE_TP
	uintptr_t canary;
	uintptr_t *dtv;
#endif

// XXX Emscripten: Need some custom thread control structures.
#ifdef __EMSCRIPTEN__
	// If --threadprofiler is enabled, this pointer is allocated to contain
	// internal information about the thread state for profiling purposes.
	thread_profiler_block * _Atomic profilerBlock;
	// The TLS base to use the main module TLS data.  Secondary modules
	// still require dynamic allocation.
	void* tls_base;
	// The lowest level of the proxying system. Other threads can enqueue
	// messages on the mailbox and notify this thread to asynchronously
	// process them once it returns to its event loop. When this thread is
	// shut down, the mailbox is closed (see below) to prevent further
	// messages from being enqueued and all the remaining queued messages
	// are dequeued and their shutdown handlers are executed. This allows
	// other threads waiting for their messages to be processed to be
	// notified that their messages will not be processed after all.
	em_task_queue* mailbox;
	// To ensure that no other thread is concurrently enqueueing a message
	// when this thread shuts down, maintain an atomic refcount. Enqueueing
	// threads atomically increment the count from a nonzero number to
	// acquire the mailbox and decrement the count when they finish. When
	// this thread shuts down it will atomically decrement the count and
	// wait until it reaches 0, at which point the mailbox is considered
	// closed and no further messages will be enqueued.
	_Atomic int mailbox_refcount;
	// Whether the thread has executed an `Atomics.waitAsync` on this
	// pthread struct and can be notified of new mailbox messages via
	// `Atomics.notify`. Otherwise, such as when the environment does not
	// implement `Atomics.waitAsync` or when the thread has not had a chance
	// to initialize itself yet, the notification has to fall back to the
	// postMessage path. Once this becomes true, it remains true so we never
	// fall back to postMessage unnecessarily.
	_Atomic int waiting_async;
#endif
#ifdef EMSCRIPTEN_DYNAMIC_LINKING
	// When dynamic linking is enabled, threads use this to facilitate the
	// synchronization of loaded code between threads.
	// See emscripten_futex_wait.c.
	_Atomic char sleeping;
#endif
};

enum {
	DT_EXITED,
	DT_EXITING,
	DT_JOINABLE,
	DT_DETACHED,
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

#ifndef TP_OFFSET
#define TP_OFFSET 0
#endif

#ifndef DTP_OFFSET
#define DTP_OFFSET 0
#endif

#ifdef TLS_ABOVE_TP
#define TP_ADJ(p) ((char *)(p) + sizeof(struct pthread) + TP_OFFSET)
#define __pthread_self() ((pthread_t)(__get_tp() - sizeof(struct __pthread) - TP_OFFSET))
#else
#define TP_ADJ(p) (p)
#define __pthread_self() ((pthread_t)__get_tp())
#endif

#ifndef tls_mod_off_t
#define tls_mod_off_t size_t
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

void *__tls_get_addr(tls_mod_off_t *);
hidden int __init_tp(void *);
hidden void *__copy_tls(unsigned char *);
hidden void __reset_tls();

hidden void __membarrier_init(void);
hidden void __dl_thread_cleanup(void);
hidden void __testcancel();
hidden void __do_cleanup_push(struct __ptcb *);
hidden void __do_cleanup_pop(struct __ptcb *);
hidden void __pthread_tsd_run_dtors();

hidden void __pthread_key_delete_synccall(void (*)(void *), void *);
hidden int __pthread_key_delete_impl(pthread_key_t);

extern hidden volatile size_t __pthread_tsd_size;
extern hidden void *__pthread_tsd_main[];
extern hidden volatile int __eintr_valid_flag;

hidden int __clone(int (*)(void *), void *, int, void *, ...);
hidden int __set_thread_area(void *);
hidden int __libc_sigaction(int, const struct sigaction *, struct sigaction *);
hidden void __unmapself(void *, size_t);

hidden int __timedwait(volatile int *, int, clockid_t, const struct timespec *, int);
hidden int __timedwait_cp(volatile int *, int, clockid_t, const struct timespec *, int);
hidden void __wait(volatile int *, volatile int *, int, int);
static inline void __wake(volatile void *addr, int cnt, int priv)
{
	if (priv) priv = FUTEX_PRIVATE;
	if (cnt<0) cnt = INT_MAX;
#ifdef __EMSCRIPTEN__
	emscripten_futex_wake(addr, (cnt)<0?INT_MAX:(cnt));
#else
	__syscall(SYS_futex, addr, FUTEX_WAKE|priv, cnt) != -ENOSYS ||
	__syscall(SYS_futex, addr, FUTEX_WAKE, cnt);
#endif
}
static inline void __futexwait(volatile void *addr, int val, int priv)
{
#ifdef __EMSCRIPTEN__
	__wait(addr, NULL, val, priv);
#else
	if (priv) priv = FUTEX_PRIVATE;
	__syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, 0) != -ENOSYS ||
	__syscall(SYS_futex, addr, FUTEX_WAIT, val, 0);
#endif
}

hidden void __acquire_ptc(void);
hidden void __release_ptc(void);
hidden void __inhibit_ptc(void);

hidden void __tl_lock(void);
hidden void __tl_unlock(void);
hidden void __tl_sync(pthread_t);

extern hidden volatile int __thread_list_lock;

extern hidden volatile int __abort_lock[1];

extern hidden unsigned __default_stacksize;
extern hidden unsigned __default_guardsize;


#ifdef __EMSCRIPTEN__
// Keep in sync with DEFAULT_PTHREAD_STACK_SIZE in settings.js
#define DEFAULT_STACK_SIZE (64*1024)
#else
#define DEFAULT_STACK_SIZE 131072
#endif
#define DEFAULT_GUARD_SIZE 8192

#define DEFAULT_STACK_MAX (8<<20)
#define DEFAULT_GUARD_MAX (1<<20)

#define __ATTRP_C11_THREAD ((void*)(uintptr_t)-1)

#endif
