#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "syscall.h"

char *getcwd(char *buf, size_t size)
{
	char tmp[PATH_MAX];
	if (!buf) {
		buf = tmp;
		size = PATH_MAX;
	} else if (!size) {
		errno = EINVAL;
		return 0;
	}
	if (syscall(SYS_getcwd, buf, size) < 0) return 0;
	return buf == tmp ? strdup(buf) : buf;
}
