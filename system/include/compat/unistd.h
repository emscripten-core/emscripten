#ifndef _COMPAT_UNISTD_H
#define _COMPAT_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif

char *  getwd(char *__buf );

#ifdef __cplusplus
}
#endif

#include_next <unistd.h>

#endif
