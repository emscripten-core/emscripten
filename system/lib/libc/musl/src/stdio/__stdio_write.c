#include "stdio_impl.h"
#include <sys/uio.h>
#include <pthread.h>

static void cleanup(void *p)
{
	FILE *f = p;
	if (!f->lockcount) __unlockfile(f);
}

size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len)
{
	struct iovec iovs[2] = {
		{ .iov_base = f->wbase, .iov_len = f->wpos-f->wbase },
		{ .iov_base = (void *)buf, .iov_len = len }
	};
	struct iovec *iov = iovs;
	size_t rem = iov[0].iov_len + iov[1].iov_len;
	int iovcnt = 2;
	ssize_t cnt;
	for (;;) {
		if (libc.main_thread) {
			pthread_cleanup_push(cleanup, f);
			cnt = syscall_cp(SYS_writev, f->fd, iov, iovcnt);
			pthread_cleanup_pop(0);
		} else {
			cnt = syscall(SYS_writev, f->fd, iov, iovcnt);
		}
		if (cnt == rem) {
			f->wend = f->buf + f->buf_size;
			f->wpos = f->wbase = f->buf;
			return len;
		}
		if (cnt < 0) {
			f->wpos = f->wbase = f->wend = 0;
			f->flags |= F_ERR;
			return iovcnt == 2 ? 0 : len-iov[0].iov_len;
		}
		rem -= cnt;
		if (cnt > iov[0].iov_len) {
			f->wpos = f->wbase = f->buf;
			cnt -= iov[0].iov_len;
			iov++; iovcnt--;
		} else if (iovcnt == 2) {
			f->wbase += cnt;
		}
		iov[0].iov_base = (char *)iov[0].iov_base + cnt;
		iov[0].iov_len -= cnt;
	}
}
