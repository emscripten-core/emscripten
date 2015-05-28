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
#include <stdarg.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "../internal/pthread_impl.h"
#include <assert.h>

// With LLVM 3.6, C11 is the default compilation mode.
// gets() is deprecated under that standard, but emcc
// still provides it, so always include it in the build.
#if __STDC_VERSION__ >= 201112L
char *gets(char *);
#endif

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

double _pthread_msecs_until(const struct timespec *restrict at)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	double cur_t = t.tv_sec * 1e3 + t.tv_usec * 1e-3;
	double at_t = at->tv_sec * 1e3 + at->tv_nsec * 1e-6;
	double msecs = at_t - cur_t;
	return msecs;
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
				double msecs = INFINITY;
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					// Sleep in small slices so that we can test cancellation to honor PTHREAD_CANCEL_ASYNCHRONOUS.
					pthread_testcancel();
					msecs = 100;
				}
				emscripten_futex_wait(&mutex->_m_addr, 2, msecs);
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
				double msecs = _pthread_msecs_until(at);
				if (msecs <= 0) return ETIMEDOUT;

				// Sleep in small slices if thread type is PTHREAD_CANCEL_ASYNCHRONOUS
				// so that we can honor PTHREAD_CANCEL_ASYNCHRONOUS requests.
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					pthread_testcancel();
					if (msecs > 100) msecs = 100;
				}
				int ret = emscripten_futex_wait(&mutex->_m_addr, 2, msecs);
				if (ret == 0) break;
				else if (threadCancelType != PTHREAD_CANCEL_ASYNCHRONOUS || _pthread_msecs_until(at) <= 0) {
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

int pthread_getattr_np(pthread_t t, pthread_attr_t *a)
{
	*a = (pthread_attr_t){0};
	a->_a_detach = !!t->detached;
	a->_a_stackaddr = (uintptr_t)t->stack;
	a->_a_stacksize = t->stack_size - DEFAULT_STACK_SIZE;
	return 0;
}

static uint32_t dummyZeroAddress = 0;

int usleep(unsigned usec)
{
	double now = emscripten_get_now();
	double target = now + usec * 1e-3;
	while(now < target) {
		double msecsToSleep = target - now;
		if (msecsToSleep > 1.0) {
			if (msecsToSleep > 100.0) msecsToSleep = 100.0;
			pthread_testcancel(); // pthreads spec: usleep is a cancellation point, so it must test if this thread is cancelled during the sleep.
			if (emscripten_is_main_runtime_thread()) emscripten_main_thread_process_queued_calls(); // Assist other threads by executing proxied operations that are effectively singlethreaded.
			emscripten_futex_wait(&dummyZeroAddress, 0, msecsToSleep);
		}
		now = emscripten_get_now();
	}
	return 0;
}

static void _do_call(em_queued_call *q)
{
	switch(q->function)
	{
		case EM_PROXIED_FOPEN: q->returnValue.vp = (void*)fopen(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_FGETS: q->returnValue.cp = fgets(q->args[0].cp, q->args[1].i, (FILE*)q->args[2].vp); break;
		case EM_PROXIED_FPUTS: q->returnValue.i = fputs(q->args[0].cp, (FILE*)q->args[1].vp); break;
		case EM_PROXIED_FCLOSE: q->returnValue.i = fclose((FILE*)q->args[0].vp); break;
		case EM_PROXIED_OPENDIR: q->returnValue.vp = opendir(q->args[0].cp); break;
		case EM_PROXIED_CLOSEDIR: q->returnValue.i = closedir((DIR*)q->args[0].vp); break;
		case EM_PROXIED_TELLDIR: q->returnValue.i = telldir((DIR*)q->args[0].vp); break;
		case EM_PROXIED_SEEKDIR: seekdir((DIR*)q->args[0].vp, q->args[1].i); break;
		case EM_PROXIED_REWINDDIR: rewinddir((DIR*)q->args[0].vp); break;
		case EM_PROXIED_READDIR_R: q->returnValue.i = readdir_r((DIR*)q->args[0].vp, (struct dirent*)q->args[1].vp, (struct dirent**)q->args[2].vp); break;
		case EM_PROXIED_READDIR: q->returnValue.vp = readdir((DIR*)q->args[0].vp); break;
		case EM_PROXIED_UTIME: q->returnValue.i = utime(q->args[0].cp, (struct utimbuf*)q->args[1].vp); break;
		case EM_PROXIED_UTIMES: q->returnValue.i = utimes(q->args[0].cp, (struct timeval*)q->args[1].vp); break;
		case EM_PROXIED_STAT: q->returnValue.i = stat(q->args[0].cp, (struct stat*)q->args[1].vp); break;
		case EM_PROXIED_LSTAT: q->returnValue.i = lstat(q->args[0].cp, (struct stat*)q->args[1].vp); break;
		case EM_PROXIED_FSTAT: q->returnValue.i = fstat(q->args[0].i, (struct stat*)q->args[1].vp); break;
		case EM_PROXIED_MKNOD: q->returnValue.i = mknod(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_MKDIR: q->returnValue.i = mkdir(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_MKFIFO: q->returnValue.i = mkfifo(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_CHMOD: q->returnValue.i = chmod(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_FCHMOD: q->returnValue.i = fchmod(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_LCHMOD: q->returnValue.i = lchmod(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_UMASK: q->returnValue.i = umask(q->args[0].i); break;
		case EM_PROXIED_STATVFS: q->returnValue.i = statvfs(q->args[0].cp, (struct statvfs*)q->args[1].vp); break;
		case EM_PROXIED_FSTATVFS: q->returnValue.i = fstatvfs(q->args[0].i, (struct statvfs*)q->args[1].vp); break;
		case EM_PROXIED_OPEN: q->returnValue.i = open(q->args[0].cp, q->args[1].i, q->args[2].vp); break;
		case EM_PROXIED_CREAT: q->returnValue.i = creat(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_MKTEMP: q->returnValue.cp = mktemp(q->args[0].cp); break;
		case EM_PROXIED_MKSTEMP: q->returnValue.i = mkstemp(q->args[0].cp); break;
		case EM_PROXIED_MKDTEMP: q->returnValue.cp = mkdtemp(q->args[0].cp); break;
		case EM_PROXIED_FCNTL: q->returnValue.i = fcntl(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_POSIX_FALLOCATE: q->returnValue.i = posix_fallocate(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_POLL: q->returnValue.i = poll((struct pollfd*)q->args[0].vp, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_ACCESS: q->returnValue.i = access(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_CHDIR: q->returnValue.i = chdir(q->args[0].cp); break;
		case EM_PROXIED_CHOWN: q->returnValue.i = chown(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_CHROOT: q->returnValue.i = chroot(q->args[0].cp); break;
		case EM_PROXIED_CLOSE: q->returnValue.i = close(q->args[0].i); break;
		case EM_PROXIED_DUP: q->returnValue.i = dup(q->args[0].i); break;
		case EM_PROXIED_DUP2: q->returnValue.i = dup2(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_FCHOWN: q->returnValue.i = fchown(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_FCHDIR: q->returnValue.i = fchdir(q->args[0].i); break;
		case EM_PROXIED_CTERMID: q->returnValue.cp = ctermid(q->args[0].cp); break;
		case EM_PROXIED_CRYPT: q->returnValue.cp = crypt(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_ENCRYPT: encrypt(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_FPATHCONF: q->returnValue.i = fpathconf(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_FSYNC: q->returnValue.i = fsync(q->args[0].i); break;
		case EM_PROXIED_TRUNCATE: q->returnValue.i = truncate(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_FTRUNCATE: q->returnValue.i = ftruncate(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_GETCWD: q->returnValue.cp = getcwd(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_ISATTY: q->returnValue.i = isatty(q->args[0].i); break;
		case EM_PROXIED_LCHOWN: q->returnValue.i = lchown(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_LINK: q->returnValue.i = link(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_LOCKF: q->returnValue.i = lockf(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_LSEEK: q->returnValue.i = lseek(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_PIPE: q->returnValue.i = pipe((int*)q->args[0].vp); break;
		case EM_PROXIED_PREAD: q->returnValue.i = pread(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[2].i); break;
		case EM_PROXIED_READ: q->returnValue.i = read(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_PROXIED_RMDIR: q->returnValue.i = rmdir(q->args[0].cp); break;
		case EM_PROXIED_UNLINK: q->returnValue.i = unlink(q->args[0].cp); break;
		case EM_PROXIED_TTYNAME: q->returnValue.cp = ttyname(q->args[0].i); break;
		case EM_PROXIED_TTYNAME_R: q->returnValue.i = ttyname_r(q->args[0].i, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_SYMLINK: q->returnValue.i = symlink(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_READLINK: q->returnValue.i = readlink(q->args[0].cp, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_PWRITE: q->returnValue.i = pwrite(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i); break;
		case EM_PROXIED_WRITE: q->returnValue.i = write(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_PROXIED_CONFSTR: q->returnValue.i = confstr(q->args[0].i, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_GETHOSTNAME: q->returnValue.i = gethostname(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_GETLOGIN: q->returnValue.cp = getlogin(); break;
		case EM_PROXIED_GETLOGIN_R: q->returnValue.i = getlogin_r(q->args[0].cp, q->args[1].i); break;
		case EM_PROXIED_SYSCONF: q->returnValue.i = sysconf(q->args[0].i); break;
		case EM_PROXIED_SBRK: q->returnValue.vp = sbrk(q->args[0].i); break;
		case EM_PROXIED_CLEARERR: clearerr(q->args[0].vp); break;
		case EM_PROXIED_FDOPEN: q->returnValue.vp = fdopen(q->args[0].i, q->args[1].vp); break;
		case EM_PROXIED_FEOF: q->returnValue.i = feof(q->args[0].vp); break;
		case EM_PROXIED_FERROR: q->returnValue.i = ferror(q->args[0].vp); break;
		case EM_PROXIED_FFLUSH: q->returnValue.i = fflush(q->args[0].vp); break;
		case EM_PROXIED_FGETC: q->returnValue.i = fgetc(q->args[0].vp); break;
		case EM_PROXIED_GETCHAR: q->returnValue.i = getchar(); break;
		case EM_PROXIED_FGETPOS: q->returnValue.i = fgetpos(q->args[0].vp, q->args[1].vp); break;
		case EM_PROXIED_GETS: q->returnValue.cp = gets(q->args[0].cp); break;
		case EM_PROXIED_FILENO: q->returnValue.i = fileno(q->args[0].vp); break;
		case EM_PROXIED_FPUTC: q->returnValue.i = fputc(q->args[0].i, q->args[1].vp); break;
		case EM_PROXIED_PUTCHAR: q->returnValue.i = putchar(q->args[0].i); break;
		case EM_PROXIED_PUTS: q->returnValue.i = puts(q->args[0].cp); break;
		case EM_PROXIED_FREAD: q->returnValue.i = fread(q->args[0].vp, q->args[1].i, q->args[2].i, q->args[3].vp); break;
		case EM_PROXIED_FREOPEN: q->returnValue.vp = freopen(q->args[0].cp, q->args[1].cp, q->args[2].vp); break;
		case EM_PROXIED_FSEEK: q->returnValue.i = fseek(q->args[0].vp, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_FSETPOS: q->returnValue.i = fsetpos(q->args[0].vp, q->args[1].vp); break;
		case EM_PROXIED_FTELL: q->returnValue.i = ftell(q->args[0].vp); break;
		case EM_PROXIED_FWRITE: q->returnValue.i = fwrite(q->args[0].vp, q->args[1].i, q->args[2].i, q->args[3].vp); break;
		case EM_PROXIED_POPEN: q->returnValue.vp = popen(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_PCLOSE: q->returnValue.i = pclose(q->args[0].vp); break;
		case EM_PROXIED_PERROR: perror(q->args[0].cp); break;
		case EM_PROXIED_REMOVE: q->returnValue.i = remove(q->args[0].cp); break;
		case EM_PROXIED_RENAME: q->returnValue.i = rename(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_REWIND: rewind(q->args[0].vp); break;
		case EM_PROXIED_TMPNAM: q->returnValue.cp = tmpnam(q->args[0].cp); break;
		case EM_PROXIED_TEMPNAM: q->returnValue.cp = tempnam(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_TMPFILE: q->returnValue.vp = tmpfile(); break;
		case EM_PROXIED_UNGETC: q->returnValue.i = ungetc(q->args[0].i, q->args[1].vp); break;
		case EM_PROXIED_FSCANF: q->returnValue.i = fscanf(q->args[0].vp, q->args[1].cp, q->args[2].vp); break;
		case EM_PROXIED_SCANF: q->returnValue.i = scanf(q->args[0].cp, q->args[1].vp); break;
		case EM_PROXIED_FPRINTF: q->returnValue.i = fprintf(q->args[0].vp, q->args[1].cp); break;
		case EM_PROXIED_PRINTF: q->returnValue.i = printf(q->args[0].cp); break;
		case EM_PROXIED_DPRINTF: q->returnValue.i = dprintf(q->args[0].i, q->args[1].cp); break;
		case EM_PROXIED_MMAP: q->returnValue.vp = mmap(q->args[0].vp, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i); break;
		case EM_PROXIED_MUNMAP: q->returnValue.i = munmap(q->args[0].vp, q->args[1].i); break;
		case EM_PROXIED_ATEXIT: q->returnValue.i = atexit(q->args[0].vp); break;
		case EM_PROXIED_GETENV: q->returnValue.cp = getenv(q->args[0].cp); break;
		case EM_PROXIED_CLEARENV: q->returnValue.i = clearenv(); break;
		case EM_PROXIED_SETENV: q->returnValue.i = setenv(q->args[0].cp, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_UNSETENV: q->returnValue.i = unsetenv(q->args[0].cp); break;
		case EM_PROXIED_PUTENV: q->returnValue.i = putenv(q->args[0].cp); break;
		case EM_PROXIED_REALPATH: q->returnValue.cp = realpath(q->args[0].cp, q->args[1].cp); break;
		case EM_PROXIED_TCGETATTR: q->returnValue.i = tcgetattr(q->args[0].i, q->args[1].vp); break;
		case EM_PROXIED_TCSETATTR: q->returnValue.i = tcsetattr(q->args[0].i, q->args[1].i, q->args[2].vp); break;
		case EM_PROXIED_TZSET: tzset(); break;
		case EM_PROXIED_SOCKET: q->returnValue.i = socket(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_PROXIED_BIND: q->returnValue.i = bind(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_PROXIED_SENDMSG: q->returnValue.i = sendmsg(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_PROXIED_RECVMSG: q->returnValue.i = recvmsg(q->args[0].i, q->args[1].vp, q->args[2].i); break;
		case EM_PROXIED_SHUTDOWN: q->returnValue.i = shutdown(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_IOCTL: q->returnValue.i = ioctl(q->args[0].i, q->args[1].i, q->args[2].vp); break;
		case EM_PROXIED_ACCEPT: q->returnValue.i = accept(q->args[0].i, q->args[1].vp, q->args[2].vp); break;
		case EM_PROXIED_SELECT: q->returnValue.i = select(q->args[0].i, q->args[1].vp, q->args[2].vp, q->args[3].vp, q->args[4].vp); break;
		case EM_PROXIED_CONNECT: q->returnValue.i = connect(q->args[0].i, q->args[1].vp, q->args[1].i); break;
		case EM_PROXIED_LISTEN: q->returnValue.i = listen(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_GETSOCKNAME: q->returnValue.i = getsockname(q->args[0].i, q->args[1].vp, q->args[2].vp); break;
		case EM_PROXIED_GETPEERNAME: q->returnValue.i = getpeername(q->args[0].i, q->args[1].vp, q->args[2].vp); break;
		case EM_PROXIED_SEND: q->returnValue.i = send(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i); break;
		case EM_PROXIED_RECV: q->returnValue.i = recv(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i); break;
		case EM_PROXIED_SENDTO: q->returnValue.i = sendto(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i, q->args[4].vp, q->args[5].i); break;
		case EM_PROXIED_RECVFROM: q->returnValue.i = recvfrom(q->args[0].i, q->args[1].vp, q->args[2].i, q->args[3].i, q->args[4].vp, q->args[5].vp); break;
		case EM_PROXIED_GETSOCKOPT: q->returnValue.i = getsockopt(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].vp, q->args[4].vp); break;
		case EM_PROXIED_PTHREAD_CREATE: q->returnValue.i = pthread_create(q->args[0].vp, q->args[1].vp, q->args[2].vp, q->args[3].vp); break;
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

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_xprintf_varargs(int function, int param0, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	const int CAP = 128;
	char str[CAP];
	char *s = str;
	int len = vsnprintf(s, CAP, format, args);
	if (len >= CAP)
	{
		s = (char*)malloc(len+1);
		va_start(args, format);
		len = vsnprintf(s, len+1, format, args);
	}
	em_queued_call q = { function, 0 };
	if (function == EM_PROXIED_PRINTF)
	{
		q.args[0].vp = s;
	}
	else
	{
		q.args[0].vp = param0;
		q.args[1].vp = s;
	}
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	if (s != str) free(s);
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

static int bool_inside_nested_process_queued_calls = 0;

void EMSCRIPTEN_KEEPALIVE emscripten_main_thread_process_queued_calls()
{
	assert(emscripten_is_main_runtime_thread() && "emscripten_main_thread_process_queued_calls must be called from the main thread!");
	if (!emscripten_is_main_runtime_thread()) return;

	// It is possible that when processing a queued call, the call flow leads back to calling this function in a nested fashion!
	// Therefore this scenario must explicitly be detected, and processing the queue must be avoided if we are nesting, or otherwise
	// the same queued calls would be processed again and again.
	if (bool_inside_nested_process_queued_calls) return;
	// This must be before pthread_mutex_lock(), since pthread_mutex_lock() can call back to this function.
	bool_inside_nested_process_queued_calls = 1;
	pthread_mutex_lock(&call_queue_lock);
	for (int i = 0; i < call_queue_length; ++i)
		_do_call(call_queue[i]);
	call_queue_length = 0;
	bool_inside_nested_process_queued_calls = 0;
	pthread_mutex_unlock(&call_queue_lock);
}

float EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_f32(const void *addr)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.u = emscripten_atomic_load_u32(addr);
	return u.f;
}

// Use an array of multiple interleaved spinlock mutexes to separate memory addresses to ease pressure when locking.
// This is outright horrible, but enables easily porting code that does require 64-bit atomics.
// Eventually in the long run we'd hope to have real support for 64-bit atomics in the browser, after
// which this emulation can be removed.
#define NUM_64BIT_LOCKS 256
static int emulated64BitAtomicsLocks[NUM_64BIT_LOCKS] = {};

uint32_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_exchange_u32(void/*uint32_t*/ *addr, uint32_t newVal)
{
	uint32_t oldVal, oldVal2;
	do {
		oldVal = emscripten_atomic_load_u32(addr);
		oldVal2 = emscripten_atomic_cas_u32(addr, oldVal, newVal);
	} while (oldVal != oldVal2);
	return oldVal;
}

#define SPINLOCK_ACQUIRE(addr) do { while(emscripten_atomic_exchange_u32((void*)(addr), 1)) /*nop*/; } while(0)
#define SPINLOCK_RELEASE(addr) emscripten_atomic_store_u32((void*)(addr), 0)

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_exchange_u64(void/*uint64_t*/ *addr, uint64_t newVal)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldValInMemory = *(uint64_t*)addr;
	*(uint64_t*)addr = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldValInMemory;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_cas_u64(void/*uint64_t*/ *addr, uint64_t oldVal, uint64_t newVal)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldValInMemory = *(uint64_t*)addr;
	if (oldValInMemory == oldVal)
		*(uint64_t*)addr = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldValInMemory;
}

double EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_f64(const void *addr)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	double val = *(double*)addr;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_u64(const void *addr)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t val = *(uint64_t*)addr;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

float EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_f32(void *addr, float val)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.f = val;
	return emscripten_atomic_store_u32(addr, u.u);
}

double EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_f64(void *addr, double val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	double *a = (double*)addr;
	*a = val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t*)addr;
	*a = val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_add_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a + val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_add. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_add_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal + val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_sub_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a - val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_sub. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_sub_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal - val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_and_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a & val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_and. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_and_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal & val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_or_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a | val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_or. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_or_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal | val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_xor_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a ^ val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_xor. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_xor_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal ^ val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}
