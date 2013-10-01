#ifndef	_SYS_MMAN_H
#define	_SYS_MMAN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#define __NEED_mode_t
#define __NEED_size_t
#define __NEED_off_t

#if defined(_GNU_SOURCE)
#define __NEED_ssize_t
#endif

#include <bits/alltypes.h>

#include <bits/mman.h>

void *mmap (void *, size_t, int, int, int, off_t);
int munmap (void *, size_t);

int mprotect (void *, size_t, int);
int msync (void *, size_t, int);

int posix_madvise (void *, size_t, int);

int mlock (const void *, size_t);
int munlock (const void *, size_t);
int mlockall (int);
int munlockall (void);

#ifdef _GNU_SOURCE
void *mremap (void *, size_t, size_t, int, ...);
int remap_file_pages (void *, size_t, int, ssize_t, int);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int madvise (void *, size_t, int);
int mincore (void *, size_t, unsigned char *);
#endif

int shm_open (const char *, int, mode_t);
int shm_unlink (const char *);

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define mmap64 mmap
#define off64_t off_t
#endif

#ifdef __cplusplus
}
#endif
#endif
