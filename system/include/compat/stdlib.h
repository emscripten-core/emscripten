#ifndef _COMPAT_STDLIB_H
#define _COMPAT_STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

int getloadavg(double loadavg[], int nelem);

#ifdef __cplusplus
}
#endif

#include_next <stdlib.h>

#endif
