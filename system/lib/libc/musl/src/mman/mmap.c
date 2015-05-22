#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include "syscall.h"
#include "libc.h"

static void dummy1(int x) { }
static void dummy0(void) { }
weak_alias(dummy1, __vm_lock);
weak_alias(dummy0, __vm_unlock);

#define OFF_MASK ((-0x2000ULL << (8*sizeof(long)-1)) | 0xfff)

void *__mmap(void *start, size_t len, int prot, int flags, int fd, off_t off)
{
	void *ret;

	if (off & OFF_MASK) {
		errno = EINVAL;
		return MAP_FAILED;
	}
	if (len >= PTRDIFF_MAX) {
		errno = ENOMEM;
		return MAP_FAILED;
	}
	if (flags & MAP_FIXED) __vm_lock(-1);
#ifdef SYS_mmap2
	ret = (void *)syscall(SYS_mmap2, start, len, prot, flags, fd, off>>12);
#else
	ret = (void *)syscall(SYS_mmap, start, len, prot, flags, fd, off);
#endif
	if (flags & MAP_FIXED) __vm_unlock();
	return ret;
}

weak_alias(__mmap, mmap);

LFS64(mmap);
