#define _GNU_SOURCE
#include "pthread_impl.h"
#include "stdio_impl.h"
#include "libc.h"
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>

void *__mmap(void *, size_t, int, int, int, off_t);
int __munmap(void *, size_t);
int __mprotect(void *, size_t, int);

static void dummy_0()
{
}
weak_alias(dummy_0, __acquire_ptc);
weak_alias(dummy_0, __release_ptc);
weak_alias(dummy_0, __pthread_tsd_run_dtors);
weak_alias(dummy_0, __do_orphaned_stdio_locks);
weak_alias(dummy_0, __dl_thread_cleanup);

_Noreturn void __pthread_exit(void *result)
{
	pthread_t self = __pthread_self();
	sigset_t set;

	self->canceldisable = 1;
	self->cancelasync = 0;
	self->result = result;

	while (self->cancelbuf) {
		void (*f)(void *) = self->cancelbuf->__f;
		void *x = self->cancelbuf->__x;
		self->cancelbuf = self->cancelbuf->__next;
		f(x);
	}

	__pthread_tsd_run_dtors();

	__lock(self->exitlock);

	/* Mark this thread dead before decrementing count */
	__lock(self->killlock);
	self->dead = 1;

	/* Block all signals before decrementing the live thread count.
	 * This is important to ensure that dynamically allocated TLS
	 * is not under-allocated/over-committed, and possibly for other
	 * reasons as well. */
	__block_all_sigs(&set);

	/* Wait to unlock the kill lock, which governs functions like
	 * pthread_kill which target a thread id, until signals have
	 * been blocked. This precludes observation of the thread id
	 * as a live thread (with application code running in it) after
	 * the thread was reported dead by ESRCH being returned. */
	__unlock(self->killlock);

	/* It's impossible to determine whether this is "the last thread"
	 * until performing the atomic decrement, since multiple threads
	 * could exit at the same time. For the last thread, revert the
	 * decrement and unblock signals to give the atexit handlers and
	 * stdio cleanup code a consistent state. */
	if (a_fetch_add(&libc.threads_minus_1, -1)==0) {
		libc.threads_minus_1 = 0;
		__restore_sigs(&set);
		exit(0);
	}

	/* Process robust list in userspace to handle non-pshared mutexes
	 * and the detached thread case where the robust list head will
	 * be invalid when the kernel would process it. */
	__vm_lock();
	volatile void *volatile *rp;
	while ((rp=self->robust_list.head) && rp != &self->robust_list.head) {
		pthread_mutex_t *m = (void *)((char *)rp
			- offsetof(pthread_mutex_t, _m_next));
		int waiters = m->_m_waiters;
		int priv = (m->_m_type & 128) ^ 128;
		self->robust_list.pending = rp;
		self->robust_list.head = *rp;
		int cont = a_swap(&m->_m_lock, 0x40000000);
		self->robust_list.pending = 0;
		if (cont < 0 || waiters)
			__wake(&m->_m_lock, 1, priv);
	}
	__vm_unlock();

	__do_orphaned_stdio_locks();
	__dl_thread_cleanup();

	if (self->detached && self->map_base) {
		/* Detached threads must avoid the kernel clear_child_tid
		 * feature, since the virtual address will have been
		 * unmapped and possibly already reused by a new mapping
		 * at the time the kernel would perform the write. In
		 * the case of threads that started out detached, the
		 * initial clone flags are correct, but if the thread was
		 * detached later (== 2), we need to clear it here. */
		if (self->detached == 2) __syscall(SYS_set_tid_address, 0);

		/* Robust list will no longer be valid, and was already
		 * processed above, so unregister it with the kernel. */
		if (self->robust_list.off)
			__syscall(SYS_set_robust_list, 0, 3*sizeof(long));

		/* Since __unmapself bypasses the normal munmap code path,
		 * explicitly wait for vmlock holders first. */
		__vm_wait();

		/* The following call unmaps the thread's stack mapping
		 * and then exits without touching the stack. */
		__unmapself(self->map_base, self->map_size);
	}

	for (;;) __syscall(SYS_exit, 0);
}

void __do_cleanup_push(struct __ptcb *cb)
{
	struct pthread *self = __pthread_self();
	cb->__next = self->cancelbuf;
	self->cancelbuf = cb;
}

void __do_cleanup_pop(struct __ptcb *cb)
{
	__pthread_self()->cancelbuf = cb->__next;
}

static int start(void *p)
{
	pthread_t self = p;
	if (self->startlock[0]) {
		__wait(self->startlock, 0, 1, 1);
		if (self->startlock[0]) {
			self->detached = 2;
			pthread_exit(0);
		}
		__restore_sigs(self->sigmask);
	}
	if (self->unblock_cancel)
		__syscall(SYS_rt_sigprocmask, SIG_UNBLOCK,
			SIGPT_SET, 0, _NSIG/8);
	__pthread_exit(self->start(self->start_arg));
	return 0;
}

static int start_c11(void *p)
{
	pthread_t self = p;
	int (*start)(void*) = (int(*)(void*)) self->start;
	__pthread_exit((void *)(uintptr_t)start(self->start_arg));
	return 0;
}

#define ROUND(x) (((x)+PAGE_SIZE-1)&-PAGE_SIZE)

/* pthread_key_create.c overrides this */
static volatile size_t dummy = 0;
weak_alias(dummy, __pthread_tsd_size);
static void *dummy_tsd[1] = { 0 };
weak_alias(dummy_tsd, __pthread_tsd_main);

volatile int __block_new_threads = 0;

static FILE *volatile dummy_file = 0;
weak_alias(dummy_file, __stdin_used);
weak_alias(dummy_file, __stdout_used);
weak_alias(dummy_file, __stderr_used);

static void init_file_lock(FILE *f)
{
	if (f && f->lock<0) f->lock = 0;
}

void *__copy_tls(unsigned char *);

int __pthread_create(pthread_t *restrict res, const pthread_attr_t *restrict attrp, void *(*entry)(void *), void *restrict arg)
{
	int ret, c11 = (attrp == __ATTRP_C11_THREAD);
	size_t size, guard;
	struct pthread *self, *new;
	unsigned char *map = 0, *stack = 0, *tsd = 0, *stack_limit;
	unsigned flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND
		| CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS
		| CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | CLONE_DETACHED;
	int do_sched = 0;
	pthread_attr_t attr = {0};

	if (!libc.can_do_threads) return ENOSYS;
	self = __pthread_self();
	if (!libc.threaded) {
		for (FILE *f=*__ofl_lock(); f; f=f->next)
			init_file_lock(f);
		__ofl_unlock();
		init_file_lock(__stdin_used);
		init_file_lock(__stdout_used);
		init_file_lock(__stderr_used);
		__syscall(SYS_rt_sigprocmask, SIG_UNBLOCK, SIGPT_SET, 0, _NSIG/8);
		self->tsd = (void **)__pthread_tsd_main;
		libc.threaded = 1;
	}
	if (attrp && !c11) attr = *attrp;

	__acquire_ptc();
	if (__block_new_threads) __wait(&__block_new_threads, 0, 1, 1);

	if (attr._a_stackaddr) {
		size_t need = libc.tls_size + __pthread_tsd_size;
		size = attr._a_stacksize + DEFAULT_STACK_SIZE;
		stack = (void *)(attr._a_stackaddr & -16);
		stack_limit = (void *)(attr._a_stackaddr - size);
		/* Use application-provided stack for TLS only when
		 * it does not take more than ~12% or 2k of the
		 * application's stack space. */
		if (need < size/8 && need < 2048) {
			tsd = stack - __pthread_tsd_size;
			stack = tsd - libc.tls_size;
			memset(stack, 0, need);
		} else {
			size = ROUND(need);
			guard = 0;
		}
	} else {
		guard = ROUND(DEFAULT_GUARD_SIZE + attr._a_guardsize);
		size = guard + ROUND(DEFAULT_STACK_SIZE + attr._a_stacksize
			+ libc.tls_size +  __pthread_tsd_size);
	}

	if (!tsd) {
		if (guard) {
			map = __mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
			if (map == MAP_FAILED) goto fail;
			if (__mprotect(map+guard, size-guard, PROT_READ|PROT_WRITE)
			    && errno != ENOSYS) {
				__munmap(map, size);
				goto fail;
			}
		} else {
			map = __mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
			if (map == MAP_FAILED) goto fail;
		}
		tsd = map + size - __pthread_tsd_size;
		if (!stack) {
			stack = tsd - libc.tls_size;
			stack_limit = map + guard;
		}
	}

	new = __copy_tls(tsd - libc.tls_size);
	new->map_base = map;
	new->map_size = size;
	new->stack = stack;
	new->stack_size = stack - stack_limit;
	new->start = entry;
	new->start_arg = arg;
	new->self = new;
	new->tsd = (void *)tsd;
	new->locale = &libc.global_locale;
	if (attr._a_detach) {
		new->detached = 1;
		flags -= CLONE_CHILD_CLEARTID;
	}
	if (attr._a_sched) {
		do_sched = new->startlock[0] = 1;
		__block_app_sigs(new->sigmask);
	}
	new->robust_list.head = &new->robust_list.head;
	new->unblock_cancel = self->cancel;
	new->CANARY = self->CANARY;

	a_inc(&libc.threads_minus_1);
	ret = __clone((c11 ? start_c11 : start), stack, flags, new, &new->tid, TP_ADJ(new), &new->tid);

	__release_ptc();

	if (do_sched) {
		__restore_sigs(new->sigmask);
	}

	if (ret < 0) {
		a_dec(&libc.threads_minus_1);
		if (map) __munmap(map, size);
		return EAGAIN;
	}

	if (do_sched) {
		ret = __syscall(SYS_sched_setscheduler, new->tid,
			attr._a_policy, &attr._a_prio);
		a_store(new->startlock, ret<0 ? 2 : 0);
		__wake(new->startlock, 1, 1);
		if (ret < 0) return -ret;
	}

	*res = new;
	return 0;
fail:
	__release_ptc();
	return EAGAIN;
}

weak_alias(__pthread_exit, pthread_exit);
weak_alias(__pthread_create, pthread_create);
