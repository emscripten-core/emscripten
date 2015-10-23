#include "stdio_impl.h"
#include <sys/uio.h>
#include <pthread.h>

static void cleanup(void *p)
{
	FILE *f = p;
	if (!f->lockcount) __unlockfile(f);
}

size_t __stdio_read(FILE *f, unsigned char *buf, size_t len)
{
	struct iovec iov[2] = {
		{ .iov_base = buf, .iov_len = len - !!f->buf_size },
		{ .iov_base = f->buf, .iov_len = f->buf_size }
	};
	ssize_t cnt;

	if (libc.main_thread) {
		pthread_cleanup_push(cleanup, f);
		cnt = syscall_cp(SYS_readv, f->fd, iov, 2);
		pthread_cleanup_pop(0);
	} else {
		cnt = syscall(SYS_readv, f->fd, iov, 2);
	}
	if (cnt <= 0) {
		f->flags |= F_EOF ^ ((F_ERR^F_EOF) & cnt);
		f->rpos = f->rend = 0;
		return cnt;
	}
	if (cnt <= iov[0].iov_len) return cnt;
	cnt -= iov[0].iov_len;
	f->rpos = f->buf;
	f->rend = f->buf + cnt;
	if (f->buf_size) buf[len-1] = *f->rpos++;
	return len;
}
