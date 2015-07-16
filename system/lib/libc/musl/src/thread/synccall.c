#include "pthread_impl.h"
#include <semaphore.h>

static struct chain {
	struct chain *next;
	sem_t sem, sem2;
} *head, *cur;

static void (*callback)(void *), *context;
static int chainlen;
static sem_t chainlock, chaindone;

static void handler(int sig, siginfo_t *si, void *ctx)
{
	struct chain ch;
	pthread_t self = __pthread_self();
	int old_errno = errno;

	if (chainlen == libc.threads_minus_1) return;

	sigqueue(self->pid, SIGSYNCCALL, (union sigval){0});

	sem_init(&ch.sem, 0, 0);
	sem_init(&ch.sem2, 0, 0);

	while (sem_wait(&chainlock));
	ch.next = head;
	head = &ch;
	if (++chainlen == libc.threads_minus_1) sem_post(&chaindone);
	sem_post(&chainlock);

	while (sem_wait(&ch.sem));
	callback(context);
	sem_post(&ch.sem2);
	while (sem_wait(&ch.sem));

	errno = old_errno;
}

void __synccall(void (*func)(void *), void *ctx)
{
	pthread_t self;
	struct sigaction sa;
	struct chain *next;
	sigset_t oldmask;

	if (!libc.threads_minus_1) {
		func(ctx);
		return;
	}

	__inhibit_ptc();

	__block_all_sigs(&oldmask);

	sem_init(&chaindone, 0, 0);
	sem_init(&chainlock, 0, 1);
	chainlen = 0;
	head = 0;
	callback = func;
	context = ctx;

	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sa.sa_sigaction = handler;
	sigfillset(&sa.sa_mask);
	__libc_sigaction(SIGSYNCCALL, &sa, 0);

	self = __pthread_self();
	sigqueue(self->pid, SIGSYNCCALL, (union sigval){0});
	while (sem_wait(&chaindone));

	sa.sa_flags = 0;
	sa.sa_handler = SIG_IGN;
	__libc_sigaction(SIGSYNCCALL, &sa, 0);

	for (cur=head; cur; cur=cur->next) {
		sem_post(&cur->sem);
		while (sem_wait(&cur->sem2));
	}
	func(ctx);

	for (cur=head; cur; cur=next) {
		next = cur->next;
		sem_post(&cur->sem);
	}

	__restore_sigs(&oldmask);

	__release_ptc();
}
