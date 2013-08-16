#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>

int main(void)
{
    int fd;
    struct statfs fs1, fs2;
    struct statvfs vfs1, vfs2;

    fd = open("/", O_RDONLY);
    assert(fd >= 0);

    statfs("/", &fs1);
    statvfs("/", &vfs1);
    fstatfs(fd, &fs2);
    fstatvfs(fd, &vfs2);

    close(fd);

    assert(fs1.f_type == fs2.f_type);
    assert(fs1.f_bsize == fs2.f_bsize);
    assert(fs1.f_blocks == fs2.f_blocks);
    assert(fs1.f_bfree == fs2.f_bfree);
    assert(fs1.f_bavail == fs2.f_bavail);
    assert(fs1.f_files == fs2.f_files);
    assert(fs1.f_ffree == fs2.f_ffree);
    assert(fs1.f_namelen == fs2.f_namelen);
    assert(fs1.f_frsize == fs2.f_frsize);

    assert(vfs1.f_bsize == vfs2.f_bsize);
    assert(vfs1.f_frsize == vfs2.f_frsize);
    assert(vfs1.f_blocks == vfs2.f_blocks);
    assert(vfs1.f_bfree == vfs2.f_bfree);
    assert(vfs1.f_bavail == vfs2.f_bavail);
    assert(vfs1.f_files == vfs2.f_files);
    assert(vfs1.f_ffree == vfs2.f_ffree);
    assert(vfs1.f_fsid == vfs2.f_fsid);
    assert(vfs1.f_flag == vfs2.f_flag);
    assert(vfs1.f_namemax == vfs2.f_namemax);

    assert(fs1.f_bsize == vfs1.f_bsize);
    assert(fs1.f_blocks == vfs1.f_blocks);
    assert(fs1.f_bfree == vfs1.f_bfree);
    assert(fs1.f_bavail == vfs1.f_bavail);
    assert(fs1.f_files == vfs1.f_files);
    assert(fs1.f_ffree == vfs1.f_ffree);
    assert(fs1.f_namelen == vfs1.f_namemax);
    assert(fs1.f_frsize == vfs1.f_frsize);

    puts("success");

    return 0;
}
