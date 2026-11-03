#include <unistd.h>
#include "syscall.h"

int pause(void)
{
	return sys_pause_cp();
}
