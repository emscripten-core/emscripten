#ifndef _COMPAT_STRING_H
#define _COMPAT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

extern char* strlwr(char *);
extern char* strupr(char *);

#ifdef __cplusplus
}
#endif

#include_next <string.h>

#endif
