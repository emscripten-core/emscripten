#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "pthread_impl.h"
#include "emscripten_internal.h"

int system(const char *cmd)
{
	return __syscall_ret(_emscripten_system(cmd));
}
