#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "syscall.h"

char *getcwd(char *buf, size_t size)
{
	char tmp[buf ? 1 : PATH_MAX];
	if (!buf) {
		buf = tmp;
		size = sizeof tmp;
	} else if (!size) {
		errno = EINVAL;
		return 0;
	}
	long ret = syscall(SYS_getcwd, buf, size);
	if (ret < 0)
		return 0;
#ifdef __EMSCRIPTEN__
	// In upstream musl, `buf[0] != '/'` checks for Linux's `(unreachable)`
	// prefix returned by the kernel's getcwd syscall. Emscripten's
	// __syscall_getcwd never returns `(unreachable)` (it returns -ENOENT
	// directly), and under NODERAWFS on Windows valid paths can start with
	// a drive letter (e.g. `C:\...`) or UNC prefix (`\\...`). That is, the
	// Emscripten change here is to get musl to work properly on Windows (which
	// musl does not normally do).
	if (ret == 0) {
#else
	if (ret == 0 || buf[0] != '/') {
#endif
		errno = ENOENT;
		return 0;
	}
	return buf == tmp ? strdup(buf) : buf;
}
