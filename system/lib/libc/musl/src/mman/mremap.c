#include <unistd.h>
#include <sys/mman.h>
#include <stdarg.h>
#include "syscall.h"
#include "libc.h"

void *__mremap(void *old_addr, size_t old_len, size_t new_len, int flags, ...)
{
	va_list ap;
	void *new_addr;
	
	va_start(ap, flags);
	new_addr = va_arg(ap, void *);
	va_end(ap);

	return (void *)syscall(SYS_mremap, old_addr, old_len, new_len, flags, new_addr);
}

weak_alias(__mremap, mremap);
