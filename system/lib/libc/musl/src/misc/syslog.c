#include <stdarg.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "libc.h"
#include "atomic.h"

static int lock[2];
static char log_ident[32];
static int log_opt;
static int log_facility = LOG_USER;
static int log_mask = 0xff;
static int log_fd = -1;

int setlogmask(int maskpri)
{
	if (maskpri) return a_swap(&log_mask, maskpri);
	else return log_mask;
}

static const struct {
	short sun_family;
	char sun_path[9];
} log_addr = {
	AF_UNIX,
	"/dev/log"
};

void closelog(void)
{
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);
	close(log_fd);
	log_fd = -1;
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

static void __openlog()
{
	int fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0);
	if (fd < 0) return;
	if (connect(fd, (void *)&log_addr, sizeof log_addr) < 0)
		close(fd);
	else
		log_fd = fd;
}

void openlog(const char *ident, int opt, int facility)
{
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);

	if (ident) {
		size_t n = strnlen(ident, sizeof log_ident - 1);
		memcpy(log_ident, ident, n);
		log_ident[n] = 0;
	} else {
		log_ident[0] = 0;
	}
	log_opt = opt;
	log_facility = facility;

	if ((opt & LOG_NDELAY) && log_fd<0) __openlog();

	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

static void _vsyslog(int priority, const char *message, va_list ap)
{
	char timebuf[16];
	time_t now;
	struct tm tm;
	char buf[256];
	int errno_save = errno;
	int pid;
	int l, l2;

	if (log_fd < 0) {
		__openlog();
		if (log_fd < 0) return;
	}

	if (!(priority & LOG_FACMASK)) priority |= log_facility;

	now = time(NULL);
	gmtime_r(&now, &tm);
	strftime(timebuf, sizeof timebuf, "%b %e %T", &tm);

	pid = (log_opt & LOG_PID) ? getpid() : 0;
	l = snprintf(buf, sizeof buf, "<%d>%s %s%s%.0d%s: ",
		priority, timebuf, log_ident, "["+!pid, pid, "]"+!pid);
	errno = errno_save;
	l2 = vsnprintf(buf+l, sizeof buf - l, message, ap);
	if (l2 >= 0) {
		if (l2 >= sizeof buf - l) l = sizeof buf - 1;
		else l += l2;
		if (buf[l-1] != '\n') buf[l++] = '\n';
		send(log_fd, buf, l, 0);
	}
}

void __vsyslog(int priority, const char *message, va_list ap)
{
	int cs;
	if (!(log_mask & LOG_MASK(priority&7)) || (priority&~0x3ff)) return;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);
	_vsyslog(priority, message, ap);
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

void syslog(int priority, const char *message, ...)
{
	va_list ap;
	va_start(ap, message);
	__vsyslog(priority, message, ap);
	va_end(ap);
}

weak_alias(__vsyslog, vsyslog);
