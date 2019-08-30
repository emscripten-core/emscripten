#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "syscall.h"
#include "pthread_impl.h"

struct ctx {
	int fd;
	const char *filename;
	int amode;
};

static const int errors[] = {
	0, -EACCES, -ELOOP, -ENAMETOOLONG, -ENOENT, -ENOTDIR,
	-EROFS, -EBADF, -EINVAL, -ETXTBSY,
	-EFAULT, -EIO, -ENOMEM,
	-EBUSY
};

static int checker(void *p)
{
	struct ctx *c = p;
	int ret;
	int i;
	if (__syscall(SYS_setregid, __syscall(SYS_getegid), -1)
	    || __syscall(SYS_setreuid, __syscall(SYS_geteuid), -1))
		__syscall(SYS_exit, 1);
	ret = __syscall(SYS_faccessat, c->fd, c->filename, c->amode, 0);
	for (i=0; i < sizeof errors/sizeof *errors - 1 && ret!=errors[i]; i++);
	return i;
}

int faccessat(int fd, const char *filename, int amode, int flag)
{
	if (!flag || (flag==AT_EACCESS && getuid()==geteuid() && getgid()==getegid()))
		return syscall(SYS_faccessat, fd, filename, amode, flag);

	if (flag != AT_EACCESS)
		return __syscall_ret(-EINVAL);

	char stack[1024];
	sigset_t set;
	pid_t pid;
	int ret = -EBUSY;
	struct ctx c = { .fd = fd, .filename = filename, .amode = amode };

	__block_all_sigs(&set);
	
	pid = __clone(checker, stack+sizeof stack, 0, &c);
	if (pid > 0) {
		int status;
		do {
			__syscall(SYS_wait4, pid, &status, __WCLONE, 0);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		if (WIFEXITED(status))
			ret = errors[WEXITSTATUS(status)];
	}

	__restore_sigs(&set);

	return __syscall_ret(ret);
}
