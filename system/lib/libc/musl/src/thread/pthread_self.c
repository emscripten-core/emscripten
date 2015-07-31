#include "pthread_impl.h"

static struct pthread *main_thread = &(struct pthread){0};

/* pthread_key_create.c overrides this */
static const void *dummy[1] = { 0 };
weak_alias(dummy, __pthread_tsd_main);

static int init_main_thread()
{
	__syscall(SYS_rt_sigprocmask, SIG_UNBLOCK,
		SIGPT_SET, 0, _NSIG/8);
	if (__set_thread_area(TP_ADJ(main_thread)) < 0) return -1;
	main_thread->canceldisable = libc.canceldisable;
	main_thread->tsd = (void **)__pthread_tsd_main;
	main_thread->errno_ptr = __errno_location();
	main_thread->self = main_thread;
	main_thread->tid = main_thread->pid =
		__syscall(SYS_set_tid_address, &main_thread->tid);
	if (!main_thread->dtv)
		main_thread->dtv = (void *)dummy;
	libc.main_thread = main_thread;
	return 0;
}

pthread_t __pthread_self_def()
{
	static int init, failed;
	if (!init) {
		if (failed) return 0;
		if (init_main_thread() < 0) failed = 1;
		if (failed) return 0;
		init = 1;
	}
	return __pthread_self();
}

weak_alias(__pthread_self_def, pthread_self);
weak_alias(__pthread_self_def, __pthread_self_init);

void *__install_initial_tls(void *p)
{
	main_thread = p;
	return __pthread_self_def();
}
