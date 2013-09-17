#ifndef _COMPAT_MALLOC_H_
#define _COMPAT_MALLOC_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This version of struct mallinfo must match the one in
   system/lib/dlmallo.c.  */

struct mallinfo {
  int arena;    /* total space allocated from system */
  int ordblks;  /* number of non-inuse chunks */
  int smblks;   /* unused -- always zero */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* total space in mmapped regions */
  int usmblks;  /* unused -- always zero */
  int fsmblks;  /* unused -- always zero */
  int uordblks; /* total allocated space */
  int fordblks; /* total non-inuse space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

/* The routines.  */

extern struct mallinfo mallinfo(void);

extern void malloc_stats(void);

extern int mallopt(int, int);

extern size_t malloc_usable_size(void*);

/* mallopt options */

#define M_TRIM_THRESHOLD    -1
#define M_GRANULARITY       -2
#define M_MMAP_THRESHOLD    -3

#ifdef __cplusplus
}
#endif

#include_next <malloc.h>

#endif /* _COMPAT_MALLOC_H_ */
