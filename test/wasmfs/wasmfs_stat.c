/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

// FIXME: Individual test to verify fstat in isolation. May get merged with
// others later.

int main() {
  // Attempt to call fstat on an invalid fd.
  errno = 0;
  struct stat invalid;
  int result = fstat(-1, &invalid);
  assert(result == -1);
  assert(errno == EBADF);

  // Test opening a file and calling fstat.
  struct stat file;
  int fd = open("/dev/stdout/", O_WRONLY);
  assert(fd >= 0);
  assert(fstat(fd, &file) != -1);

  off_t fileInode = file.st_ino;

  assert(file.st_size == 0);
  assert((file.st_mode & S_IFMT) == S_IFCHR);
  assert(file.st_dev);
  assert(file.st_nlink);
  assert(file.st_uid == 0);
  assert(file.st_gid == 0);
#ifdef WASMFS
  assert(file.st_rdev == 0);
#endif
  assert(file.st_blocks == 0);
  assert(file.st_blksize == 4096);

  close(fd);

  // Check to see if the previous inode number matches.
  int newfd = open("/dev/stdout/", O_WRONLY);
  struct stat newFile;
  assert(newfd >= 0);
  assert(fstat(newfd, &newFile) != -1);

  assert(fileInode == newFile.st_ino);
  close(newfd);

  // Test opening a directory and calling fstat.
  struct stat directory;
  int fd2 = open("/dev", O_RDONLY | O_DIRECTORY);
  assert(fd2 >= 0);
  assert(fstat(fd2, &directory) != -1);

  off_t dirInode = directory.st_ino;

  assert(directory.st_size == 4096);
  assert((directory.st_mode & S_IFMT) == S_IFDIR);
  assert(directory.st_dev);
  assert(directory.st_nlink);
  assert(directory.st_uid == 0);
  assert(directory.st_gid == 0);
  assert(directory.st_rdev == 0);
#ifdef WASMFS
  // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
  assert(directory.st_blocks == 8);
#else
  assert(directory.st_rdev == 0);
  // The JS file system calculates st_blocks using Math.ceil(attr.size /
  // attr.blksize);
  assert(directory.st_blocks == 1);
#endif
  assert(directory.st_blksize == 4096);

  close(fd2);

  // Check to see if the previous inode number matches.
  int newfd2 = open("/dev", O_RDONLY | O_DIRECTORY);
  struct stat newFile2;
  assert(newfd2 >= 0);
  assert(fstat(newfd2, &newFile2) != -1);

  assert(dirInode == newFile2.st_ino);
  close(newfd2);

  // Test calling stat without opening a file.
  struct stat statFile;
  assert(stat("/dev/stdout/", &statFile) != -1);

  assert(statFile.st_size == 0);
  assert((statFile.st_mode & S_IFMT) == S_IFCHR);
  assert(statFile.st_dev);
  assert(statFile.st_nlink);
  assert(statFile.st_uid == 0);
  assert(statFile.st_gid == 0);
#ifdef WASMFS
  assert(statFile.st_rdev == 0);
#endif
  assert(statFile.st_blocks == 0);
  assert(statFile.st_blksize == 4096);

  // Test calling stat without opening a directory.
  struct stat statDirectory;
  assert(stat("/dev", &statDirectory) != -1);

  assert(statDirectory.st_size == 4096);
  assert((statDirectory.st_mode & S_IFMT) == S_IFDIR);
  assert(statDirectory.st_dev);
  assert(statDirectory.st_nlink);
  assert(statDirectory.st_uid == 0);
  assert(statDirectory.st_gid == 0);
  assert(statDirectory.st_rdev == 0);
#ifdef WASMFS
  // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
  assert(statDirectory.st_blocks == 8);
#else
  assert(statDirectory.st_blocks == 1);
#endif
  assert(statDirectory.st_blksize == 4096);

  // Test calling stat with an empty path.
  errno = 0;
  assert(stat("", &invalid));
  assert(errno == ENOENT);

  // Test calling stat with a non-existent path.
  errno = 0;
  assert(stat("/non-existent", &invalid));
  assert(errno == ENOENT);

  // Test calling lstat without opening a file.
  struct stat lstatFile;
  errno = 0;
  assert(lstat("/dev/stdout", &lstatFile) != -1);

  assert(lstatFile.st_dev);
  // TODO: When symlinks are added, this WasmFS should return S_IFLNK.
#ifdef WASMFS
  assert((lstatFile.st_mode & S_IFMT) == S_IFCHR);
#else
  assert((lstatFile.st_mode & S_IFMT) == S_IFLNK);
#endif
  assert(lstatFile.st_nlink);
  assert(lstatFile.st_uid == 0);
  assert(lstatFile.st_gid == 0);
  assert(lstatFile.st_blksize == 4096);
  assert(lstatFile.st_rdev == 0);
#ifdef WASMFS
  assert(lstatFile.st_size == 0);
  assert(lstatFile.st_blocks == 0);
#else
  // dev/stdout is a symlink to dev/tty.
  // TODO: When symlinks are added, one should return stat info on the symlink.
  assert(lstatFile.st_size == 8);
  assert(lstatFile.st_blocks == 1);
#endif

  // Test calling lstat without opening a directory.
  struct stat lstatDirectory;
  assert(lstat("/dev", &lstatDirectory) != -1);

  assert(lstatDirectory.st_size == 4096);
  assert((lstatDirectory.st_mode & S_IFMT) == S_IFDIR);
  assert(lstatDirectory.st_dev);
  assert(lstatDirectory.st_nlink);
  assert(lstatDirectory.st_uid == 0);
  assert(lstatDirectory.st_gid == 0);
  assert(lstatDirectory.st_rdev == 0);
#ifdef WASMFS
  // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
  assert(lstatDirectory.st_blocks == 8);
#else
  assert(lstatDirectory.st_blocks == 1);
#endif
  assert(lstatDirectory.st_blksize == 4096);

  // write to a file and check that mtime is updated
  const long long TEST_TIME = 1ll << 34;
  struct utimbuf t = {TEST_TIME, TEST_TIME};
  fd = open("file", O_WRONLY | O_CREAT | O_EXCL, 0777);
  assert(utime("file", &t) != -1);

  write(fd, 0, 0);
  assert(fstat(fd, &statFile) != -1);
  assert(statFile.st_mtime == TEST_TIME);

  const char* buffer = "abcdef";
  write(fd, buffer, sizeof(buffer));
  assert(fstat(fd, &statFile) != -1);
  assert(statFile.st_mtime != TEST_TIME);

  close(fd);

  return 0;
}
