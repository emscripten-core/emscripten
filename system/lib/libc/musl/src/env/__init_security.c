#include <elf.h>
#include <poll.h>
#include <fcntl.h>
#include "syscall.h"
#include "libc.h"
#include "atomic.h"

static void dummy(void *ent)
{
}
weak_alias(dummy, __init_ssp);

void __init_security(size_t *aux)
{
	struct pollfd pfd[3] = { {.fd=0}, {.fd=1}, {.fd=2} };
	int i;

#ifndef SHARED
	__init_ssp((void *)aux[AT_RANDOM]);
#endif

	if (aux[AT_UID]==aux[AT_EUID] && aux[AT_GID]==aux[AT_EGID]
		&& !aux[AT_SECURE]) return;

	__syscall(SYS_poll, pfd, 3, 0);
	for (i=0; i<3; i++) if (pfd[i].revents&POLLNVAL)
		if (__syscall(SYS_open, "/dev/null", O_RDWR|O_LARGEFILE)<0)
			a_crash();
	libc.secure = 1;
}
