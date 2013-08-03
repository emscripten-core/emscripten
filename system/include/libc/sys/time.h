#ifndef _SYS_TIME_H
#define _SYS_TIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#include <sys/select.h>

int gettimeofday (struct timeval *__restrict, void *__restrict);

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct itimerval
{
	struct timeval it_interval;
	struct timeval it_value;
};

int getitimer (int, struct itimerval *);
int setitimer (int, const struct itimerval *__restrict, struct itimerval *__restrict);
int utimes (const char *, const struct timeval [2]);

#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
int futimes(int, const struct timeval [2]);
int futimesat(int, const char *, const struct timeval [2]);
int lutimes(const char *, const struct timeval [2]);
int settimeofday(const struct timeval *, const struct timezone *);
int adjtime (const struct timeval *, struct timeval *);
#define timerisset(t) ((t)->tv_sec || (t)->tv_usec)
#define timerclear(t) ((t)->tv_sec = (t)->tv_usec = 0)
#define timercmp(s,t,op) ((s)->tv_sec == (t)->tv_sec ? \
	(s)->tv_usec op (t)->tv_usec : (s)->tv_sec op (t)->tv_sec)
#define timeradd(s,t,a) ( (a)->tv_sec = (s)->tv_sec + (t)->tv_sec, \
	((a)->tv_usec = (s)->tv_usec + (t)->tv_usec) >= 1000000 && \
	((a)->tv_usec -= 1000000, (a)->tv_sec++) )
#define timersub(s,t,a) ( (a)->tv_sec = (s)->tv_sec - (t)->tv_sec, \
	((a)->tv_usec = (s)->tv_usec - (t)->tv_usec) < 0 && \
	((a)->tv_usec += 1000000, (a)->tv_sec--) )
#endif

#ifdef __cplusplus
}
#endif
#endif
