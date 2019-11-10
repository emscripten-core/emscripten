#include <unistd.h>
#include <signal.h>
#include "syscall.h"
#include "libc.h"

int pause(void)
{
#ifdef SYS_pause
	return syscall_cp(SYS_pause);
#else
	return syscall_cp(SYS_ppoll, 0, 0, 0, 0);
#endif
}
