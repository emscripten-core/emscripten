#ifndef LIBC_H
#define LIBC_H

#include <stdlib.h>
#include <stdio.h>

struct __libc {
	void *main_thread;
	int threaded;
	int secure;
	size_t *auxv;
	int (*atexit)(void (*)(void));
	void (*fini)(void);
	void (*ldso_fini)(void);
	volatile int threads_minus_1;
	int canceldisable;
	FILE *ofl_head;
	int ofl_lock[2];
	size_t tls_size;
};

extern size_t __hwcap;

#if !defined(__PIC__) || (100*__GNUC__+__GNUC_MINOR__ >= 303 && !defined(__PCC__))

#ifdef __PIC__
#if __GNUC__ < 4
#define BROKEN_VISIBILITY 1
#endif
#define ATTR_LIBC_VISIBILITY __attribute__((visibility("hidden")))
#else
#define ATTR_LIBC_VISIBILITY
#endif

extern struct __libc __libc ATTR_LIBC_VISIBILITY;
#define libc __libc

#else

#define USE_LIBC_ACCESSOR
#define ATTR_LIBC_VISIBILITY
extern struct __libc *__libc_loc(void) __attribute__((const));
#define libc (*__libc_loc())

#endif


/* Designed to avoid any overhead in non-threaded processes */
void __lock(volatile int *) ATTR_LIBC_VISIBILITY;
void __unlock(volatile int *) ATTR_LIBC_VISIBILITY;
int __lockfile(FILE *) ATTR_LIBC_VISIBILITY;
void __unlockfile(FILE *) ATTR_LIBC_VISIBILITY;
#define LOCK(x) (libc.threads_minus_1 ? (__lock(x),1) : ((void)(x),1))
#define UNLOCK(x) (libc.threads_minus_1 ? (__unlock(x),1) : ((void)(x),1))

void __synccall(void (*)(void *), void *);
int __setxid(int, int, int, int);

extern char **__environ;

#undef weak_alias
#define weak_alias(old, new) \
	extern __typeof(old) new __attribute__((weak, alias(#old)))

#undef LFS64_2
#define LFS64_2(x, y) weak_alias(x, y)

#undef LFS64
#define LFS64(x) LFS64_2(x, x##64)

#endif
