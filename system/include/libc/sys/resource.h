#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include <sys/time.h>

#define	RUSAGE_SELF	0		/* calling process */
#define	RUSAGE_CHILDREN	-1		/* terminated child processes */

struct rusage {
  struct timeval ru_utime;	/* user time used */
  struct timeval ru_stime;	/* system time used */
  int            ru_maxrss; /* XXX Emscripten */
};

/* XXX Emscripten */
int getrusage(int who, struct rusage *r_usage);

/* XXX Emscripten */
#define RLIMIT_CORE 100
#define RLIMIT_CPU 1
#define RLIMIT_DATA 2
#define RLIMIT_FSIZE 3
#define RLIMIT_NOFILE 4
#define RLIMIT_STACK 5
#define RLIMIT_AS 6

typedef unsigned rlim_t;
struct rlimit {
  rlim_t rlim_cur;
  rlim_t rlim_max;
};
int setrlimit(int resource, const struct rlimit *rlim);

#endif

