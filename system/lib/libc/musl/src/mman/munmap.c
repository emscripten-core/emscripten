#include <sys/mman.h>
#include "syscall.h"
#include "libc.h"

static void dummy1(int x) { }
static void dummy0(void) { }
weak_alias(dummy1, __vm_lock);
weak_alias(dummy0, __vm_unlock);

int __munmap(void *start, size_t len)
{
	int ret;
	__vm_lock(-1);
	ret = syscall(SYS_munmap, start, len);
	__vm_unlock();
	return ret;
}

weak_alias(__munmap, munmap);
