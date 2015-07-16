#include <elf.h>
#include <limits.h>
#include <sys/mman.h>
#include <string.h>
#include "pthread_impl.h"
#include "libc.h"
#include "atomic.h"

#ifndef SHARED

struct tls_image {
	void *image;
	size_t len, size, align;
} __static_tls ATTR_LIBC_VISIBILITY;

#define T __static_tls

void *__copy_tls(unsigned char *mem)
{
	pthread_t td;
	if (!T.image) return mem;
	void **dtv = (void *)mem;
	dtv[0] = (void *)1;
#ifdef TLS_ABOVE_TP
	mem += sizeof(void *) * 2;
	mem += -((uintptr_t)mem + sizeof(struct pthread)) & (T.align-1);
	td = (pthread_t)mem;
	mem += sizeof(struct pthread);
#else
	mem += libc.tls_size - sizeof(struct pthread);
	mem -= (uintptr_t)mem & (T.align-1);
	td = (pthread_t)mem;
	mem -= T.size;
#endif
	td->dtv = dtv;
	dtv[1] = mem;
	memcpy(mem, T.image, T.len);
	return td;
}

void *__tls_get_addr(size_t *v)
{
	return (char *)__pthread_self()->dtv[1]+v[1];
}

static void *simple(void *p)
{
	*(void **)p = p;
	return __set_thread_area(TP_ADJ(p)) ? 0 : p;
}

weak_alias(simple, __install_initial_tls);

void *__mmap(void *, size_t, int, int, int, off_t);

#if ULONG_MAX == 0xffffffff
typedef Elf32_Phdr Phdr;
#else
typedef Elf64_Phdr Phdr;
#endif

void __init_tls(size_t *aux)
{
	unsigned char *p, *mem;
	size_t n;
	Phdr *phdr, *tls_phdr=0;
	size_t base = 0;

	libc.tls_size = sizeof(struct pthread);

	for (p=(void *)aux[AT_PHDR],n=aux[AT_PHNUM]; n; n--,p+=aux[AT_PHENT]) {
		phdr = (void *)p;
		if (phdr->p_type == PT_PHDR)
			base = aux[AT_PHDR] - phdr->p_vaddr;
		if (phdr->p_type == PT_TLS)
			tls_phdr = phdr;
	}
	if (!tls_phdr) return;

	T.image = (void *)(base + tls_phdr->p_vaddr);
	T.len = tls_phdr->p_filesz;
	T.size = tls_phdr->p_memsz;
	T.align = tls_phdr->p_align;

	T.size += (-T.size - (uintptr_t)T.image) & (T.align-1);
	if (T.align < 4*sizeof(size_t)) T.align = 4*sizeof(size_t);

	libc.tls_size = 2*sizeof(void *)+T.size+T.align+sizeof(struct pthread);

	mem = __mmap(0, libc.tls_size, PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (!__install_initial_tls(__copy_tls(mem))) a_crash();
}
#else
void __init_tls(size_t *auxv) { }
#endif
