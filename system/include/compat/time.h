#ifndef _COMPAT_TIME_H
#define _COMPAT_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

int dysize(int year);
#define _timezone timezone
#define _daylight daylight

#ifdef __cplusplus
}
#endif

#include_next <time.h>

#endif
