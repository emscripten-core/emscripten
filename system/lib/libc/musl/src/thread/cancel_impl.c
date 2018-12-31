#include "pthread_impl.h"
#include "syscall.h"

void __cancel()
{
	pthread_t self = __pthread_self();
	self->canceldisable = 1;
	self->cancelasync = 0;
	pthread_exit(PTHREAD_CANCELED);
}

long __syscall_cp_asm(volatile void *, syscall_arg_t,
                      syscall_arg_t, syscall_arg_t, syscall_arg_t,
                      syscall_arg_t, syscall_arg_t, syscall_arg_t);

long (__syscall_cp)(syscall_arg_t nr,
                    syscall_arg_t u, syscall_arg_t v, syscall_arg_t w,
                    syscall_arg_t x, syscall_arg_t y, syscall_arg_t z)
{
	pthread_t self;
	long r;

	if (!libc.main_thread || (self = __pthread_self())->canceldisable)
		return __syscall(nr, u, v, w, x, y, z);

	r = __syscall_cp_asm(&self->cancel, nr, u, v, w, x, y, z);
	if (r==-EINTR && nr!=SYS_close && self->cancel && !self->canceldisable)
		__cancel();
	return r;
}

static void _sigaddset(sigset_t *set, int sig)
{
	unsigned s = sig-1;
	set->__bits[s/8/sizeof *set->__bits] |= 1UL<<(s&8*sizeof *set->__bits-1);
}

static void cancel_handler(int sig, siginfo_t *si, void *ctx)
{
	pthread_t self = __pthread_self();
	ucontext_t *uc = ctx;
	const char *ip = ((char **)&uc->uc_mcontext)[CANCEL_REG_IP];
	extern const char __cp_begin[1], __cp_end[1];

	if (!self->cancel || self->canceldisable) return;

	_sigaddset(&uc->uc_sigmask, SIGCANCEL);

	if (self->cancelasync || ip >= __cp_begin && ip < __cp_end) {
		self->canceldisable = 1;
		pthread_sigmask(SIG_SETMASK, &uc->uc_sigmask, 0);
		__cancel();
	}

	__syscall(SYS_tgkill, self->pid, self->tid, SIGCANCEL);
}

void __testcancel()
{
	pthread_t self = pthread_self();
	if (self->cancel && !self->canceldisable)
		__cancel();
}

static void init_cancellation()
{
	struct sigaction sa = {
		.sa_flags = SA_SIGINFO | SA_RESTART,
		.sa_sigaction = cancel_handler
	};
	sigfillset(&sa.sa_mask);
	__libc_sigaction(SIGCANCEL, &sa, 0);
}

int pthread_cancel(pthread_t t)
{
	static int init;
	if (!init) {
		init_cancellation();
		init = 1;
	}
	a_store(&t->cancel, 1);
	return pthread_kill(t, SIGCANCEL);
}
