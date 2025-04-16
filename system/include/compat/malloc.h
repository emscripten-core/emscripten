#ifndef _COMPAT_MALLOC_H_
#define _COMPAT_MALLOC_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This version of struct mallinfo must match the one in
   system/lib/dlmalloc.c.  */

struct mallinfo {
  size_t arena;    /* total space allocated from system */
  size_t ordblks;  /* number of non-inuse chunks */
  size_t smblks;   /* unused -- always zero */
  size_t hblks;    /* number of mmapped regions */
  size_t hblkhd;   /* total space in mmapped regions */
  size_t usmblks;  /* unused -- always zero */
  size_t fsmblks;  /* unused -- always zero */
  size_t uordblks; /* total allocated space */
  size_t fordblks; /* total non-inuse space */
  size_t keepcost; /* top-most, releasable (via malloc_trim) space */
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
