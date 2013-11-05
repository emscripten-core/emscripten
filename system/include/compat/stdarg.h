#ifndef _COMPAT_STDARG_H
#define _COMPAT_STDARG_H

#ifdef __cplusplus
extern "C" {
#endif

#define __va_copy(d,s) __builtin_va_copy(d,s)

#ifdef __cplusplus
}
#endif

#include_next <stdarg.h>

#endif
