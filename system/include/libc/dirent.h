#ifndef _DIRENT_H_
#define _DIRENT_H_
#ifdef __cplusplus
extern "C" {
#endif

#if !defined(MAXNAMLEN) && !defined(_POSIX_SOURCE)
#define MAXNAMLEN 1024
#endif

#include <sys/dirent.h>

#ifdef __cplusplus
}
#endif
#endif /*_DIRENT_H_*/
