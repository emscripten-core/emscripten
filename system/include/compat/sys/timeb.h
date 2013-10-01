/* timeb.h -- An implementation of the standard Unix <sys/timeb.h> file.
   Written by Ian Lance Taylor <ian@cygnus.com>
   Public domain; no rights reserved.

   <sys/timeb.h> declares the structure used by the ftime function, as
   well as the ftime function itself.  Newlib does not provide an
   implementation of ftime.  */

#ifndef _SYS_TIMEB_H

#ifdef __cplusplus
extern "C" {
#endif

#define _SYS_TIMEB_H

#define __NEED_time_t

#include <bits/alltypes.h>

struct timeb
{
  time_t time;
  unsigned short millitm;
  short timezone;
  short dstflag;
};

extern int ftime(struct timeb *);

#ifdef __cplusplus
}
#endif

#endif /* ! defined (_SYS_TIMEB_H) */
