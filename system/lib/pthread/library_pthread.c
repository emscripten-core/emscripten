#define _GNU_SOURCE
#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten.h>
#include <sys/time.h>
#include <dirent.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "../internal/pthread_impl.h"
#include <assert.h>

int _pthread_getcanceltype()
{
	return pthread_self()->cancelasync;
}

static void inline __pthread_mutex_locked(pthread_mutex_t *mutex)
{
	// The lock is now ours, mark this thread as the owner of this lock.
	assert(mutex);
	assert(mutex->_m_lock == 0);
	mutex->_m_lock = pthread_self()->tid;
	if (_pthread_getcanceltype() == PTHREAD_CANCEL_ASYNCHRONOUS) pthread_testcancel();
}

double _pthread_nsecs_until(const struct timespec *restrict at)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	double cur_t = t.tv_sec * 1e9 + t.tv_usec * 1e3;
	double at_t = at->tv_sec * 1e9 + at->tv_nsec;
	double nsecs = at_t - cur_t;
	return nsecs;
}

#if 0
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	if (!mutex) return EINVAL;
	assert(pthread_self() != 0);
	assert(pthread_self()->tid != 0);

	if (mutex->_m_lock == pthread_self()->tid) {
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
			if ((unsigned)mutex->_m_count >= INT_MAX) return EAGAIN;
			++mutex->_m_count;
			return 0;
		} else if ((mutex->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK) {
			return EDEADLK;
		}
	}

	int threadCancelType = _pthread_getcanceltype();

	int c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1);
	if (c != 0) {
		do {
			if (c == 2 || emscripten_atomic_cas_u32(&mutex->_m_addr, 1, 2) != 0) {
				double nsecs = INFINITY;
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					// Sleep in small slices so that we can test cancellation to honor PTHREAD_CANCEL_ASYNCHRONOUS.
					pthread_testcancel();
					nsecs = 100 * 1000 * 1000;
				}
				emscripten_futex_wait(&mutex->_m_addr, 2, nsecs);
			}
		} while((c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 2)));
	}

	__pthread_mutex_locked(mutex);
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (!mutex) return EINVAL;
	assert(pthread_self() != 0);

	if (mutex->_m_type != PTHREAD_MUTEX_NORMAL) {
		if (mutex->_m_lock != pthread_self()->tid) return EPERM;
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE && mutex->_m_count) {
			--mutex->_m_count;
			return 0;
		}
	}

	mutex->_m_lock = 0;
	if (emscripten_atomic_sub_u32((uint32_t*)&mutex->_m_addr, 1) != 1)
	{
		emscripten_atomic_store_u32((uint32_t*)&mutex->_m_addr, 0);
		emscripten_futex_wake((uint32_t*)&mutex->_m_addr, 1);
	}
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if (!mutex) return EINVAL;
	if (mutex->_m_lock == pthread_self()->tid) {
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
			if ((unsigned)mutex->_m_count >= INT_MAX) return EAGAIN;
			++mutex->_m_count;
			return 0;
		} else if ((mutex->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK) {
			return EDEADLK;
		}
	}

	if (emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1) == 0) {
		__pthread_mutex_locked(mutex);
		return 0;
	}
	else
		return EBUSY;
}

int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict at)
{
	if (!mutex || !at) return EINVAL;
	if (mutex->_m_lock == pthread_self()->tid) {
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
			if ((unsigned)mutex->_m_count >= INT_MAX) return EAGAIN;
			++mutex->_m_count;
			return 0;
		} else if ((mutex->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK) {
			return EDEADLK;
		}
	}

	int threadCancelType = _pthread_getcanceltype();
	int c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1);
	if (c != 0) {
		do {
			if (c == 2 || emscripten_atomic_cas_u32(&mutex->_m_addr, 1, 2) != 0)
			{
				if (at->tv_nsec < 0 || at->tv_nsec >= 1000000000) return EINVAL;
				double nsecs = _pthread_nsecs_until(at);
				if (nsecs <= 0) return ETIMEDOUT;

				// Sleep in small slices if thread type is PTHREAD_CANCEL_ASYNCHRONOUS
				// so that we can honor PTHREAD_CANCEL_ASYNCHRONOUS requests.
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					pthread_testcancel();
					if (nsecs > 100 * 1000 * 1000) nsecs = 100 * 1000 * 1000;
				}
				int ret = emscripten_futex_wait(&mutex->_m_addr, 2, nsecs);
				if (ret == 0) break;
				else if (threadCancelType != PTHREAD_CANCEL_ASYNCHRONOUS || _pthread_nsecs_until(at) <= 0) {
					return ETIMEDOUT;
				}
			}
		} while((c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 2)));
	}

	__pthread_mutex_locked(mutex);
	return 0;
}
#endif

int sched_get_priority_max(int policy)
{
	// Web workers do not actually support prioritizing threads,
	// but mimic values that Linux apparently reports, see
	// http://man7.org/linux/man-pages/man2/sched_get_priority_min.2.html
	if (policy == SCHED_FIFO || policy == SCHED_RR)
		return 99;
	else
		return 0;
}

int sched_get_priority_min(int policy)
{
	// Web workers do not actually support prioritizing threads,
	// but mimic values that Linux apparently reports, see
	// http://man7.org/linux/man-pages/man2/sched_get_priority_min.2.html
	if (policy == SCHED_FIFO || policy == SCHED_RR)
		return 1;
	else
		return 0;
}

int pthread_setcancelstate(int new, int *old)
{
	if (new > 1U) return EINVAL;
	struct pthread *self = pthread_self();
	if (old) *old = self->canceldisable;
	self->canceldisable = new;
	return 0;
}

int _pthread_isduecanceled(struct pthread *pthread_ptr)
{
	return pthread_ptr->threadStatus == 2/*canceled*/;
}

void pthread_testcancel()
{
	struct pthread *self = pthread_self();
	if (self->canceldisable) return;
	if (_pthread_isduecanceled(self)) {
		EM_ASM( throw 'Canceled!'; );
	}
}

static uint32_t dummyZeroAddress = 0;

int usleep(unsigned usec)
{
	double now = emscripten_get_now();
	double target = now + usec / 1000.0;
	while(now < target) {
		double nsecsToSleep = (target - now) * 1e6;
		if (nsecsToSleep > 1e6) {
			if (nsecsToSleep > 100 * 1000 * 1000) nsecsToSleep = 100 * 1000 * 1000;
			pthread_testcancel(); // pthreads spec: usleep is a cancellation point, so it must test if this thread is cancelled during the sleep.
			emscripten_futex_wait(&dummyZeroAddress, 0, nsecsToSleep);
		}
		now = emscripten_get_now();
	}
	return 0;
}

static void _do_call(em_queued_call *q)
{
	switch(q->function)
	{
		case EM_DEFERRED_FOPEN: q->returnValue.vp = (void*)fopen(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_FGETS: q->returnValue.cp = fgets(q->args[0].cp, q->args[1].i, (FILE*)q->args[2].vp); break;
		case EM_DEFERRED_FPUTS: q->returnValue.i = fputs(q->args[0].cp, (FILE*)q->args[1].vp); break;
		case EM_DEFERRED_FCLOSE: q->returnValue.i = fclose((FILE*)q->args[0].vp); break;
		case EM_DEFERRED_OPENDIR: q->returnValue.vp = opendir(q->args[0].cp); break;
		case EM_DEFERRED_CLOSEDIR: q->returnValue.i = closedir((DIR*)q->args[0].vp); break;
		case EM_DEFERRED_TELLDIR: q->returnValue.i = telldir((DIR*)q->args[0].vp); break;
		case EM_DEFERRED_SEEKDIR: seekdir((DIR*)q->args[0].vp, q->args[1].i); break;
		case EM_DEFERRED_REWINDDIR: rewinddir((DIR*)q->args[0].vp); break;
		case EM_DEFERRED_READDIR_R: q->returnValue.i = readdir_r((DIR*)q->args[0].vp, (struct dirent*)q->args[1].vp, (struct dirent**)q->args[2].vp); break;
		case EM_DEFERRED_READDIR: q->returnValue.vp = readdir((DIR*)q->args[0].vp); break;
		case EM_DEFERRED_UTIME: q->returnValue.i = utime(q->args[0].cp, (struct utimbuf*)q->args[1].vp); break;
		case EM_DEFERRED_UTIMES: q->returnValue.i = utimes(q->args[0].cp, (struct timeval*)q->args[1].vp); break;
		case EM_DEFERRED_STAT: q->returnValue.i = stat(q->args[0].cp, (struct stat*)q->args[1].vp); break;
		case EM_DEFERRED_LSTAT: q->returnValue.i = lstat(q->args[0].cp, (struct stat*)q->args[1].vp); break;
		case EM_DEFERRED_FSTAT: q->returnValue.i = fstat(q->args[0].i, (struct stat*)q->args[1].vp); break;
		case EM_DEFERRED_MKNOD: q->returnValue.i = mknod(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_MKDIR: q->returnValue.i = mkdir(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_MKFIFO: q->returnValue.i = mkfifo(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_CHMOD: q->returnValue.i = chmod(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_FCHMOD: q->returnValue.i = fchmod(q->args[0].i, q->args[1].i); break;
		case EM_DEFERRED_LCHMOD: q->returnValue.i = lchmod(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_UMASK: q->returnValue.i = umask(q->args[0].i); break;
		case EM_DEFERRED_STATVFS: q->returnValue.i = statvfs(q->args[0].cp, (struct statvfs*)q->args[1].vp); break;
		case EM_DEFERRED_FSTATVFS: q->returnValue.i = fstatvfs(q->args[0].i, (struct statvfs*)q->args[1].vp); break;
		case EM_DEFERRED_OPEN: q->returnValue.i = open(q->args[0].cp, q->args[1].i, q->args[2].vp); break;
		case EM_DEFERRED_CREAT: q->returnValue.i = creat(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_MKTEMP: q->returnValue.cp = mktemp(q->args[0].cp); break;
		case EM_DEFERRED_MKSTEMP: q->returnValue.i = mkstemp(q->args[0].cp); break;
		case EM_DEFERRED_MKDTEMP: q->returnValue.cp = mkdtemp(q->args[0].cp); break;
		case EM_DEFERRED_FCNTL: q->returnValue.i = fcntl(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_POSIX_FALLOCATE: q->returnValue.i = posix_fallocate(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_POLL: q->returnValue.i = poll((struct pollfd*)q->args[0].vp, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_ACCESS: q->returnValue.i = access(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_CHDIR: q->returnValue.i = chdir(q->args[0].cp); break;
		case EM_DEFERRED_CHOWN: q->returnValue.i = chown(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_CHROOT: q->returnValue.i = chroot(q->args[0].cp); break;
		case EM_DEFERRED_CLOSE: q->returnValue.i = close(q->args[0].i); break;
		case EM_DEFERRED_DUP: q->returnValue.i = dup(q->args[0].i); break;
		case EM_DEFERRED_DUP2: q->returnValue.i = dup2(q->args[0].i, q->args[1].i); break;
		case EM_DEFERRED_FCHOWN: q->returnValue.i = fchown(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_FCHDIR: q->returnValue.i = fchdir(q->args[0].i); break;
		case EM_DEFERRED_CTERMID: q->returnValue.cp = ctermid(q->args[0].cp); break;
		case EM_DEFERRED_CRYPT: q->returnValue.cp = crypt(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_ENCRYPT: encrypt(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_FPATHCONF: q->returnValue.i = fpathconf(q->args[0].i, q->args[1].i); break;
		case EM_DEFERRED_FSYNC: q->returnValue.i = fsync(q->args[0].i); break;
		case EM_DEFERRED_TRUNCATE: q->returnValue.i = truncate(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_FTRUNCATE: q->returnValue.i = ftruncate(q->args[0].i, q->args[1].i); break;
		case EM_DEFERRED_GETCWD: q->returnValue.cp = getcwd(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_GETWD: q->returnValue.cp = getwd(q->args[0].cp); break;
		case EM_DEFERRED_ISATTY: q->returnValue.i = isatty(q->args[0].i); break;
		case EM_DEFERRED_LCHOWN: q->returnValue.i = lchown(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_LINK: q->returnValue.i = link(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_LOCKF: q->returnValue.i = lockf(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_LSEEK: q->returnValue.i = lseek(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_PIPE: q->returnValue.i = pipe((int*)q->args[0].vp); break;
		case EM_DEFERRED_PREAD: q->returnValue.i = pread(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[2].i); break;
		case EM_DEFERRED_READ: q->returnValue.i = read(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_DEFERRED_RMDIR: q->returnValue.i = rmdir(q->args[0].cp); break;
		case EM_DEFERRED_UNLINK: q->returnValue.i = unlink(q->args[0].cp); break;
		case EM_DEFERRED_TTYNAME: q->returnValue.cp = ttyname(q->args[0].i); break;
		case EM_DEFERRED_TTYNAME_R: q->returnValue.i = ttyname_r(q->args[0].i, q->args[1].cp, q->args[2].i); break;
		case EM_DEFERRED_SYMLINK: q->returnValue.i = symlink(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_READLINK: q->returnValue.i = readlink(q->args[0].cp, q->args[1].cp, q->args[2].i); break;
		case EM_DEFERRED_PWRITE: q->returnValue.i = pwrite(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i); break;
		case EM_DEFERRED_WRITE: q->returnValue.i = write(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_DEFERRED_CONFSTR: q->returnValue.i = confstr(q->args[0].i, q->args[1].cp, q->args[2].i); break;
		case EM_DEFERRED_GETHOSTNAME: q->returnValue.i = gethostname(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_GETLOGIN: q->returnValue.cp = getlogin(); break;
		case EM_DEFERRED_GETLOGIN_R: q->returnValue.i = getlogin_r(q->args[0].cp, q->args[1].i); break;
		case EM_DEFERRED_SYSCONF: q->returnValue.i = sysconf(q->args[0].i); break;
		case EM_DEFERRED_SBRK: q->returnValue.vp = sbrk(q->args[0].i); break;
		case EM_DEFERRED_CLEARERR: clearerr(q->args[0].vp); break;
		case EM_DEFERRED_FDOPEN: q->returnValue.vp = fdopen(q->args[0].i, q->args[1].vp); break;
		case EM_DEFERRED_FEOF: q->returnValue.i = feof(q->args[0].vp); break;
		case EM_DEFERRED_FERROR: q->returnValue.i = ferror(q->args[0].vp); break;
		case EM_DEFERRED_FFLUSH: q->returnValue.i = fflush(q->args[0].vp); break;
		case EM_DEFERRED_FGETC: q->returnValue.i = fgetc(q->args[0].vp); break;
		case EM_DEFERRED_GETCHAR: q->returnValue.i = getchar(); break;
		case EM_DEFERRED_FGETPOS: q->returnValue.i = fgetpos(q->args[0].vp, q->args[1].vp); break;
		case EM_DEFERRED_GETS: q->returnValue.cp = gets(q->args[0].cp); break;
		case EM_DEFERRED_FILENO: q->returnValue.i = fileno(q->args[0].vp); break;
		case EM_DEFERRED_FPUTC: q->returnValue.i = fputc(q->args[0].i, q->args[1].vp); break;
		case EM_DEFERRED_PUTCHAR: q->returnValue.i = putchar(q->args[0].i); break;
		case EM_DEFERRED_PUTS: q->returnValue.i = putc(q->args[0].i, q->args[1].vp); break;
		case EM_DEFERRED_FREAD: q->returnValue.i = fread(q->args[0].vp, q->args[1].i, q->args[2].i, q->args[3].vp); break;
		case EM_DEFERRED_FREOPEN: q->returnValue.vp = freopen(q->args[0].cp, q->args[1].cp, q->args[2].vp); break;
		case EM_DEFERRED_FSEEK: q->returnValue.i = fseek(q->args[0].vp, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_FSETPOS: q->returnValue.i = fsetpos(q->args[0].vp, q->args[1].vp); break;
		case EM_DEFERRED_FTELL: q->returnValue.i = ftell(q->args[0].vp); break;
		case EM_DEFERRED_FWRITE: q->returnValue.i = fwrite(q->args[0].vp, q->args[1].i, q->args[2].i, q->args[3].vp); break;
		case EM_DEFERRED_POPEN: q->returnValue.vp = popen(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_PCLOSE: q->returnValue.i = pclose(q->args[0].vp); break;
		case EM_DEFERRED_PERROR: perror(q->args[0].cp); break;
		case EM_DEFERRED_REMOVE: q->returnValue.i = remove(q->args[0].cp); break;
		case EM_DEFERRED_RENAME: q->returnValue.i = rename(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_REWIND: rewind(q->args[0].vp); break;
		case EM_DEFERRED_TMPNAM: q->returnValue.cp = tmpnam(q->args[0].cp); break;
		case EM_DEFERRED_TEMPNAM: q->returnValue.cp = tempnam(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_TMPFILE: q->returnValue.vp = tmpfile(); break;
		case EM_DEFERRED_UNGETC: q->returnValue.i = ungetc(q->args[0].i, q->args[1].vp); break;
		case EM_DEFERRED_FSCANF: q->returnValue.i = fscanf(q->args[0].vp, q->args[1].cp, q->args[2].vp); break;
		case EM_DEFERRED_SCANF: q->returnValue.i = scanf(q->args[0].cp, q->args[1].vp); break;
		case EM_DEFERRED_FPRINTF: q->returnValue.i = fprintf(q->args[0].vp, q->args[1].cp, q->args[2].vp); break;
		case EM_DEFERRED_PRINTF: q->returnValue.i = printf(q->args[0].cp, q->args[1].vp); break;
		case EM_DEFERRED_DPRINTF: q->returnValue.i = dprintf(q->args[0].i, q->args[1].cp, q->args[2].vp); break;
		case EM_DEFERRED_MMAP: q->returnValue.vp = mmap(q->args[0].vp, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i); break;
		case EM_DEFERRED_MUNMAP: q->returnValue.i = munmap(q->args[0].vp, q->args[1].i); break;
		case EM_DEFERRED_ATEXIT: q->returnValue.i = atexit(q->args[0].vp); break;
		case EM_DEFERRED_GETENV: q->returnValue.cp = getenv(q->args[0].cp); break;
		case EM_DEFERRED_CLEARENV: q->returnValue.i = clearenv(); break;
		case EM_DEFERRED_SETENV: q->returnValue.i = setenv(q->args[0].cp, q->args[1].cp, q->args[2].i); break;
		case EM_DEFERRED_UNSETENV: q->returnValue.i = unsetenv(q->args[0].cp); break;
		case EM_DEFERRED_PUTENV: q->returnValue.i = putenv(q->args[0].cp); break;
		case EM_DEFERRED_REALPATH: q->returnValue.cp = realpath(q->args[0].cp, q->args[1].cp); break;
		case EM_DEFERRED_TCGETATTR: q->returnValue.i = tcgetattr(q->args[0].i, q->args[1].vp); break;
		case EM_DEFERRED_TCSETATTR: q->returnValue.i = tcsetattr(q->args[0].i, q->args[1].i, q->args[2].vp); break;
		case EM_DEFERRED_TZSET: tzset(); break;
		case EM_DEFERRED_SOCKET: q->returnValue.i = socket(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_DEFERRED_BIND: q->returnValue.i = bind(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_DEFERRED_SENDMSG: q->returnValue.i = sendmsg(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_DEFERRED_RECVMSG: q->returnValue.i = recvmsg(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_DEFERRED_SHUTDOWN: q->returnValue.i = shutdown(q->args[0].i, q->args[1].i); break;
		case EM_DEFERRED_IOCTL: q->returnValue.i = ioctl(q->args[0].i, q->args[1].i, q->args[2].vp); break;
		case EM_DEFERRED_ACCEPT: q->returnValue.i = accept(q->args[0].i, q->args[1].vp, q->args[2].vp); break;
		case EM_DEFERRED_SELECT: q->returnValue.i = select(q->args[0].i, q->args[1].vp, q->args[2].vp, q->args[3].vp, q->args[4].vp); break;
		case EM_DEFERRED_CONNECT: q->returnValue.i = connect(q->args[0].i, q->args[1].vp, q->args[1].i); break;
		case EM_DEFERRED_LISTEN: q->returnValue.i = listen(q->args[0].i, q->args[1].i); break;
		case EM_DEFERRED_GETSOCKNAME: q->returnValue.i = getsockname(q->args[0].i, q->args[1].vp, q->args[2].vp); break;
		case EM_DEFERRED_GETPEERNAME: q->returnValue.i = getpeername(q->args[0].i, q->args[1].vp, q->args[2].vp); break;
		case EM_DEFERRED_SEND: q->returnValue.i = send(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i); break;
		case EM_DEFERRED_RECV: q->returnValue.i = recv(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i); break;
		case EM_DEFERRED_SENDTO: q->returnValue.i = sendto(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i, q->args[4].vp, q->args[5].i); break;
		case EM_DEFERRED_RECVFROM: q->returnValue.i = recvfrom(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i, q->args[4].vp, q->args[5].vp); break;
		case EM_DEFERRED_GETSOCKOPT: q->returnValue.i = getsockopt(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].vp, q->args[4].vp); break;
		default: assert(0 && "Invalid Emscripten pthread _do_call opcode!");
	}
	q->operationDone = 1;
	emscripten_futex_wake(&q->operationDone, INT_MAX);
}

#define CALL_QUEUE_SIZE 128
static em_queued_call **call_queue = 0;
static int call_queue_length = 0; // Shared data synchronized by call_queue_lock.
static pthread_mutex_t call_queue_lock = PTHREAD_MUTEX_INITIALIZER;

void EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread(em_queued_call *call)
{
	assert(call);
	if (emscripten_is_main_runtime_thread()) {
		_do_call(call);
		return;
	}
	pthread_mutex_lock(&call_queue_lock);
	if (!call_queue) call_queue = malloc(sizeof(em_queued_call*) * CALL_QUEUE_SIZE); // Shared data synchronized by call_queue_lock.
	// Note: currently call_queue_length can be at most the number of pthreads that are currently running, so the queue can never get
	// full. However if/when the queue is extended to be asynchronous for void-returning functions later, this will need to be revised.
	assert(call_queue_length < CALL_QUEUE_SIZE);
	call_queue[call_queue_length] = call;
	++call_queue_length;
	if (call_queue_length == 1) {
		EM_ASM(postMessage({ cmd: 'processQueuedMainThreadWork' }));
	}
	pthread_mutex_unlock(&call_queue_lock);
	int r;
	do {
		r = emscripten_futex_wait(&call->operationDone, 0, INFINITY);
	} while(r != 0 && call->operationDone == 0);
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_1(int function, void *arg1)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_2(int function, void *arg1, void *arg2)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_3(int function, void *arg1, void *arg2, void *arg3)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_4(int function, void *arg1, void *arg2, void *arg3, void *arg4)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_5(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.args[4].vp = arg5;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_6(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.args[4].vp = arg5;
	q.args[5].vp = arg6;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void EMSCRIPTEN_KEEPALIVE emscripten_main_thread_process_queued_calls()
{
	assert(emscripten_is_main_runtime_thread() && "emscripten_main_thread_process_queued_calls must be called from the main thread!");
	if (!emscripten_is_main_runtime_thread()) return;

	pthread_mutex_lock(&call_queue_lock);
	for(int i = 0; i < call_queue_length; ++i)
		_do_call(call_queue[i]);
	call_queue_length = 0;
	pthread_mutex_unlock(&call_queue_lock);
}

float EMSCRIPTEN_KEEPALIVE _Atomics_load_HEAPF32_emulated(void *addr)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.u = emscripten_atomic_load_u32(addr);
	return u.f;
}

double EMSCRIPTEN_KEEPALIVE _Atomics_load_HEAPF64_emulated(void *addr)
{
	union {
		double d;
		uint32_t u[2];
	} u;

	for(;;) {
		u.u[0] = emscripten_atomic_load_u32(addr);
		u.u[1] = emscripten_atomic_load_u32((void*)((uintptr_t)addr + 4));
		uint32_t low2 = emscripten_atomic_load_u32(addr);
		if (u.u[0] == low2) {
			return u.d;
		}
	}
}

void EMSCRIPTEN_KEEPALIVE _Atomics_store_HEAPF32_emulated(void *addr, float val)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.f = val;
	emscripten_atomic_store_u32(addr, u.u);
}

void EMSCRIPTEN_KEEPALIVE _Atomics_store_HEAPF64_emulated(void *addr, double val)
{
	union {
		double d;
		uint32_t u[2];
	} u;
	u.d = val;

	for(;;) {
		emscripten_atomic_store_u32(addr, u.u[0]);
		emscripten_atomic_store_u32((void*)((uintptr_t)addr + 4), u.u[1]);
		uint32_t low2 = emscripten_atomic_load_u32(addr);
		if (u.u[0] == low2) {
			return;
		}
	}
}
