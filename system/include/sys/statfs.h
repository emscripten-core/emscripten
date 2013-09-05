#ifndef _SYS_STATFS_H
#define _SYS_STATFS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
# define __SWORD_TYPE           int
#else /* __WORDSIZE == 64 */
# define __SWORD_TYPE           long int
#endif

struct statfs {
    __SWORD_TYPE f_type;
    __SWORD_TYPE f_bsize;
    fsblkcnt_t   f_blocks;
    fsblkcnt_t   f_bfree;
    fsblkcnt_t   f_bavail;
    fsfilcnt_t   f_files;
    fsfilcnt_t   f_ffree;
    fsid_t       f_fsid;
    __SWORD_TYPE f_namelen;
    __SWORD_TYPE f_frsize;
    __SWORD_TYPE f_spare[5];
};

int statfs(const char *path, struct statfs *buf);
int fstatfs(int fd, struct statfs *buf);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_STATFS_H */
