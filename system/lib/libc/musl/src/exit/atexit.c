#include <stdlib.h>
#include <stdint.h>
#include "libc.h"

/* Ensure that at least 32 atexit handlers can be registered without malloc */
#define COUNT 32

static struct fl
{
	struct fl *next;
	void (*f[COUNT])(void *);
	void *a[COUNT];
} builtin, *head;

static int lock[2];

void __funcs_on_exit()
{
	int i;
	void (*func)(void *), *arg;
	LOCK(lock);
	for (; head; head=head->next) for (i=COUNT-1; i>=0; i--) {
		if (!head->f[i]) continue;
		func = head->f[i];
		arg = head->a[i];
		head->f[i] = 0;
		UNLOCK(lock);
		func(arg);
		LOCK(lock);
	}
}

void __cxa_finalize(void *dso)
{
}

int __cxa_atexit(void (*func)(void *), void *arg, void *dso)
{
	int i;

	LOCK(lock);

	/* Defer initialization of head so it can be in BSS */
	if (!head) head = &builtin;

	/* If the current function list is full, add a new one */
	if (head->f[COUNT-1]) {
		struct fl *new_fl = calloc(sizeof(struct fl), 1);
		if (!new_fl) {
			UNLOCK(lock);
			return -1;
		}
		new_fl->next = head;
		head = new_fl;
	}

	/* Append function to the list. */
	for (i=0; i<COUNT && head->f[i]; i++);
	head->f[i] = func;
	head->a[i] = arg;

	UNLOCK(lock);
	return 0;
}

static void call(void *p)
{
	((void (*)(void))(uintptr_t)p)();
}

int atexit(void (*func)(void))
{
	return __cxa_atexit(call, (void *)(uintptr_t)func, 0);
}
