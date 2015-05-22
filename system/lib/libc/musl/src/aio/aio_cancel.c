#include <aio.h>
#include <pthread.h>
#include <errno.h>

int aio_cancel(int fd, struct aiocb *cb)
{
	if (!cb) {
		/* FIXME: for correctness, we should return AIO_ALLDONE
		 * if there are no outstanding aio operations on this
		 * file descriptor, but that would require making aio
		 * much slower, and seems to have little advantage since
		 * we don't support cancellation anyway. */
		return AIO_NOTCANCELED;
	}
	return cb->__err==EINPROGRESS ? AIO_NOTCANCELED : AIO_ALLDONE;
}
