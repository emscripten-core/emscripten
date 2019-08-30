#include <aio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "pthread_impl.h"
#include "libc.h"

static void dummy(void)
{
}

weak_alias(dummy, __aio_wake);

static void notify_signal(struct sigevent *sev)
{
	siginfo_t si = {
		.si_signo = sev->sigev_signo,
		.si_value = sev->sigev_value,
		.si_code = SI_ASYNCIO,
		.si_pid = __pthread_self()->pid,
		.si_uid = getuid()
	};
	__syscall(SYS_rt_sigqueueinfo, si.si_pid, si.si_signo, &si);
}

static void *io_thread(void *p)
{
	struct aiocb *cb = p;
	int fd = cb->aio_fildes;
	void *buf = (void *)cb->aio_buf;
	size_t len = cb->aio_nbytes;
	off_t off = cb->aio_offset;
	int op = cb->aio_lio_opcode;
	struct sigevent sev = cb->aio_sigevent;
	ssize_t ret;

	if (op == LIO_WRITE) {
		if (  (fcntl(fd, F_GETFL) & O_APPEND)
		    ||((ret = pwrite(fd, buf, len, off))<0 && errno==ESPIPE) )
			ret = write(fd, buf, len);
	} else if (op == LIO_READ) {
		if ( (ret = pread(fd, buf, len, off))<0 && errno==ESPIPE )
			ret = read(fd, buf, len);
	} else {
		ret = 0;
	}
	cb->__ret = ret;

	if (ret < 0) a_store(&cb->__err, errno);
	else a_store(&cb->__err, 0);

	__aio_wake();

	switch (sev.sigev_notify) {
	case SIGEV_SIGNAL:
		notify_signal(&sev);
		break;
	case SIGEV_THREAD:
		sev.sigev_notify_function(sev.sigev_value);
		break;
	}

	return 0;
}

static int new_req(struct aiocb *cb)
{
	int ret = 0;
	pthread_attr_t a;
	sigset_t set;
	pthread_t td;

	if (cb->aio_sigevent.sigev_notify == SIGEV_THREAD) {
		if (cb->aio_sigevent.sigev_notify_attributes)
			a = *cb->aio_sigevent.sigev_notify_attributes;
		else
			pthread_attr_init(&a);
	} else {
		pthread_attr_init(&a);
		pthread_attr_setstacksize(&a, PAGE_SIZE);
		pthread_attr_setguardsize(&a, 0);
	}
	pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, &set);
	cb->__err = EINPROGRESS;
	if (pthread_create(&td, &a, io_thread, cb)) {
		errno = EAGAIN;
		ret = -1;
	}
	pthread_sigmask(SIG_SETMASK, &set, 0);
	cb->__td = td;

	return ret;
}

int aio_read(struct aiocb *cb)
{
	cb->aio_lio_opcode = LIO_READ;
	return new_req(cb);
}

int aio_write(struct aiocb *cb)
{
	cb->aio_lio_opcode = LIO_WRITE;
	return new_req(cb);
}
