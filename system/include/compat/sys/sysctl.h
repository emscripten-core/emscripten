#ifndef	_SYS_SYSCTL_H
#define	_SYS_SYSCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define __NEED_size_t
#include <bits/alltypes.h>

int sysctl (int *, int, void *, size_t *, void *, size_t);

#ifdef __cplusplus
}
#endif

#endif
