/*
 * strings.h
 *
 * Definitions for string operations.
 */

#ifndef _STRINGS_H_
#define _STRINGS_H_

#include "_ansi.h"
#include <sys/reent.h>

#include <sys/types.h> /* for size_t */

_BEGIN_STD_C

#if !defined __STRICT_ANSI__ && _POSIX_VERSION < 200809L
/* 
 * Marked LEGACY in Open Group Base Specifications Issue 6/IEEE Std 1003.1-2004
 * Removed from Open Group Base Specifications Issue 7/IEEE Std 1003.1-2008
 */
int	 _EXFUN(bcmp,(const void *, const void *, size_t));
void	 _EXFUN(bcopy,(const void *, void *, size_t));
void	 _EXFUN(bzero,(void *, size_t));
char 	*_EXFUN(index,(const char *, int));
char 	*_EXFUN(rindex,(const char *, int));
#endif /* ! __STRICT_ANSI__ */

int	 _EXFUN(ffs,(int));
int	 _EXFUN(strcasecmp,(const char *, const char *));
int	 _EXFUN(strncasecmp,(const char *, const char *, size_t));

_END_STD_C

#endif /* _STRINGS_H_ */
