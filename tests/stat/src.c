/*
Note: Hardcoded st_ino values etc. may change with minor changes to the library impl.
      In such an event, we will need to update output.txt here.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
  struct stat s;

  printf("--stat FOLDER--\n");
  printf("ret: %d\n", stat("/test", &s));
  printf("errno: %d\n", errno);
  printf("st_dev: %lu\n", s.st_dev);
  printf("st_ino: %lu\n", s.st_ino);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_nlink: %d\n", s.st_nlink);
  printf("st_rdev: %lu\n", s.st_rdev);
  printf("st_size: %ld\n", s.st_size);
  printf("st_atime: %ld\n", s.st_atime);
  printf("st_mtime: %ld\n", s.st_mtime);
  printf("st_ctime: %ld\n", s.st_ctime);
  printf("st_blksize: %ld\n", s.st_blksize);
  printf("st_blocks: %ld\n", s.st_blocks);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--stat FILE--\n");
  printf("ret: %d\n", stat("/test/file", &s));
  printf("errno: %d\n", errno);
  printf("st_dev: %lu\n", s.st_dev);
  printf("st_ino: %lu\n", s.st_ino);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_nlink: %d\n", s.st_nlink);
  printf("st_rdev: %lu\n", s.st_rdev);
  printf("st_size: %ld\n", s.st_size);
  printf("st_atime: %ld\n", s.st_atime);
  printf("st_mtime: %ld\n", s.st_mtime);
  printf("st_ctime: %ld\n", s.st_ctime);
  printf("st_blksize: %ld\n", s.st_blksize);
  printf("st_blocks: %ld\n", s.st_blocks);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--stat DEVICE--\n");
  printf("ret: %d\n", stat("/test/device", &s));
  printf("errno: %d\n", errno);
  printf("st_dev: %lu\n", s.st_dev);
  printf("st_ino: %lu\n", s.st_ino);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_nlink: %d\n", s.st_nlink);
  printf("st_rdev: %lu\n", s.st_rdev);
  printf("st_size: %ld\n", s.st_size);
  printf("st_atime: %ld\n", s.st_atime);
  printf("st_mtime: %ld\n", s.st_mtime);
  printf("st_ctime: %ld\n", s.st_ctime);
  printf("st_blksize: %ld\n", s.st_blksize);
  printf("st_blocks: %ld\n", s.st_blocks);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--stat LINK--\n");
  printf("ret: %d\n", stat("/test/link", &s));
  printf("errno: %d\n", errno);
  printf("st_dev: %lu\n", s.st_dev);
  printf("st_ino: %lu\n", s.st_ino);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_nlink: %d\n", s.st_nlink);
  printf("st_rdev: %lu\n", s.st_rdev);
  printf("st_size: %ld\n", s.st_size);
  printf("st_atime: %ld\n", s.st_atime);
  printf("st_mtime: %ld\n", s.st_mtime);
  printf("st_ctime: %ld\n", s.st_ctime);
  printf("st_blksize: %ld\n", s.st_blksize);
  printf("st_blocks: %ld\n", s.st_blocks);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--lstat LINK--\n");
  printf("ret: %d\n", lstat("/test/link", &s));
  printf("errno: %d\n", errno);
  printf("st_dev: %lu\n", s.st_dev);
  printf("st_ino: %lu\n", s.st_ino);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_nlink: %d\n", s.st_nlink);
  printf("st_rdev: %lu\n", s.st_rdev);
  printf("st_size: %ld\n", s.st_size);
  printf("st_atime: %ld\n", s.st_atime);
  printf("st_mtime: %ld\n", s.st_mtime);
  printf("st_ctime: %ld\n", s.st_ctime);
  printf("st_blksize: %ld\n", s.st_blksize);
  printf("st_blocks: %ld\n", s.st_blocks);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--fstat FILE--\n");
  printf("ret: %d\n", fstat(open("/test/file", O_RDONLY, 0777), &s));
  printf("errno: %d\n", errno);
  printf("st_dev: %lu\n", s.st_dev);
  printf("st_ino: %lu\n", s.st_ino);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_nlink: %d\n", s.st_nlink);
  printf("st_rdev: %lu\n", s.st_rdev);
  printf("st_size: %ld\n", s.st_size);
  printf("st_atime: %ld\n", s.st_atime);
  printf("st_mtime: %ld\n", s.st_mtime);
  printf("st_ctime: %ld\n", s.st_ctime);
  printf("st_blksize: %ld\n", s.st_blksize);
  printf("st_blocks: %ld\n", s.st_blocks);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--chmod FILE--\n");
  printf("ret: %d\n", chmod("/test/file", 0200));
  printf("errno: %d\n", errno);
  stat("/test/file", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_mtime changed: %d\n", s.st_mtime != 1200000000l);
  memset(&s, 0, sizeof s);

  printf("\n--fchmod FILE--\n");
  printf("ret: %d\n", fchmod(open("/test/file", O_WRONLY, 0777), 0777));
  printf("errno: %d\n", errno);
  stat("/test/file", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_mtime changed: %d\n", s.st_mtime != 1200000000l);
  memset(&s, 0, sizeof s);

  printf("\n--chmod FOLDER--\n");
  printf("ret: %d\n", chmod("/test", 0400));
  printf("errno: %d\n", errno);
  stat("/test", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("st_mtime changed: %d\n", s.st_mtime != 1200000000l);
  memset(&s, 0, sizeof s);

  printf("\n--chmod LINK--\n");
  printf("ret: %d\n", chmod("/test/link", 0000));
  printf("errno: %d\n", errno);
  stat("/test/file", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  memset(&s, 0, sizeof s);

  // Make sure we can create stuff in the root.
  chmod("/", 0777);

  printf("\n--mkdir--\n");
  printf("ret: %d\n", mkdir("/test-mkdir", 0777));
  printf("errno: %d\n", errno);
  stat("/test-mkdir", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--mknod FILE--\n");
  printf("ret: %d\n", mknod("/test-mknod-file", S_IFREG | 0777, 0));
  printf("errno: %d\n", errno);
  stat("/test-mknod-file", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--mknod FOLDER--\n");
  printf("ret: %d\n", mknod("/test-mknod-dir", S_IFDIR | 0777, 0));
  printf("errno: %d\n", errno);
  stat("/test-mknod-dir", &s);
  printf("st_mode: 0%o\n", s.st_mode);
  printf("S_ISBLK: %d\n", S_ISBLK(s.st_mode));
  printf("S_ISCHR: %d\n", S_ISCHR(s.st_mode));
  printf("S_ISDIR: %d\n", S_ISDIR(s.st_mode));
  printf("S_ISFIFO: %d\n", S_ISFIFO(s.st_mode));
  printf("S_ISREG: %d\n", S_ISREG(s.st_mode));
  printf("S_ISLNK: %d\n", S_ISLNK(s.st_mode));
  printf("S_ISSOCK: %d\n", S_ISSOCK(s.st_mode));
  memset(&s, 0, sizeof s);

  printf("\n--mknod FIFO--\n");
  printf("ret: %d\n", mknod("/test-mknod-fifo", S_IFIFO | 0777, 0));
  printf("errno: %d\n", errno);

  printf("\n--mknod DEVICE--\n");
  printf("ret: %d\n", mknod("/test-mknod-device", S_IFCHR | 0777, 123));
  printf("errno: %d\n", errno);

  printf("\n--mkfifo--\n");
  printf("ret: %d\n", mkfifo("/test-mkfifo", 0777));
  printf("errno: %d\n", errno);

  return 0;
}
