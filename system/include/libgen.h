/*
 * libgen.h - defined by XPG4
 */

#ifndef _LIBGEN_H_
#define _LIBGEN_H_

#include "_ansi.h"
#include <sys/reent.h>

#ifdef __cplusplus
extern "C" {
#endif

char      *_EXFUN(basename,     (char *));
char      *_EXFUN(dirname,     (char *));

#ifdef __cplusplus
}
#endif

#endif /* _LIBGEN_H_ */

