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
  int fd = open("/dev/stdout/", O_RDONLY);
  fstat(fd, &file);
  assert(fd >= 0);

  off_t fileInode = file.st_ino;

  assert(file.st_size == 0);
  assert(file.st_dev);
  assert(file.st_nlink);
  assert(file.st_uid == 0);
  assert(file.st_gid == 0);
  assert(file.st_rdev);
  assert(file.st_blocks == 0);
  assert(file.st_blksize == 4096);
#ifdef WASMFS
  assert(file.st_atim.tv_sec == 0);
  assert(file.st_mtim.tv_sec == 0);
  assert(file.st_ctim.tv_sec == 0);
#endif

  close(fd);

  // Check to see if the previous inode number matches.
  int newfd = open("/dev/stdout/", O_RDONLY);
  struct stat newFile;
  fstat(newfd, &newFile);

  assert(fileInode == newFile.st_ino);
  close(newfd);

  // Test opening a directory and calling fstat.
  struct stat directory;
  int fd2 = open("/dev", O_RDONLY | O_DIRECTORY);
  fstat(fd2, &directory);
  assert(fd2 >= 0);

  off_t dirInode = directory.st_ino;

  assert(directory.st_size == 4096);
  assert(directory.st_dev);
  assert(directory.st_nlink);
  assert(directory.st_uid == 0);
  assert(directory.st_gid == 0);
#ifdef WASMFS
  assert(directory.st_rdev);
  // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
  assert(directory.st_blocks == 8);
  assert(directory.st_atim.tv_sec == 0);
  assert(directory.st_mtim.tv_sec == 0);
  assert(directory.st_ctim.tv_sec == 0);
#else
  assert(!directory.st_rdev);
  assert(directory.st_blocks == 1);
#endif
  assert(directory.st_blksize == 4096);

  close(fd2);

  // Check to see if the previous inode number matches.
  int newfd2 = open("/dev", O_RDONLY | O_DIRECTORY);
  struct stat newFile2;
  fstat(newfd, &newFile2);

  assert(dirInode == newFile2.st_ino);
  close(newfd2);

  // Test opening a file and calling stat.
  struct stat statFile;
  stat("/dev/stdout/", &statFile);

  assert(statFile.st_size == 0);
  assert(statFile.st_dev);
  assert(statFile.st_nlink);
  assert(statFile.st_uid == 0);
  assert(statFile.st_gid == 0);
  assert(statFile.st_rdev);
  assert(statFile.st_blocks == 0);
  assert(statFile.st_blksize == 4096);
#ifdef WASMFS
  assert(statFile.st_atim.tv_sec == 0);
  assert(statFile.st_mtim.tv_sec == 0);
  assert(statFile.st_ctim.tv_sec == 0);
#endif

  // Test opening a directory and calling stat.
  struct stat statDirectory;
  stat("/dev", &statDirectory);

  assert(statDirectory.st_size == 4096);
  assert(statDirectory.st_dev);
  assert(statDirectory.st_nlink);
  assert(statDirectory.st_uid == 0);
  assert(statDirectory.st_gid == 0);
#ifdef WASMFS
  assert(statDirectory.st_rdev);
  // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
  assert(statDirectory.st_blocks == 8);
  assert(statDirectory.st_atim.tv_sec == 0);
  assert(statDirectory.st_mtim.tv_sec == 0);
  assert(statDirectory.st_ctim.tv_sec == 0);
#else
  assert(!statDirectory.st_rdev);
  assert(statDirectory.st_blocks == 1);
#endif
  assert(statDirectory.st_blksize == 4096);

  // Test opening a file and calling lstat.
  struct stat lstatFile;
  stat("/dev/stdout/", &lstatFile);
  assert(fd >= 0);

  assert(lstatFile.st_size == 0);
  assert(lstatFile.st_dev);
  assert(lstatFile.st_nlink);
  assert(lstatFile.st_uid == 0);
  assert(lstatFile.st_gid == 0);
  assert(lstatFile.st_rdev);
  assert(lstatFile.st_blocks == 0);
  assert(lstatFile.st_blksize == 4096);
#ifdef WASMFS
  assert(lstatFile.st_atim.tv_sec == 0);
  assert(lstatFile.st_mtim.tv_sec == 0);
  assert(lstatFile.st_ctim.tv_sec == 0);
#endif

  // Test opening a directory and calling lstat.
  struct stat lstatDirectory;
  lstat("/dev", &lstatDirectory);

  assert(lstatDirectory.st_size == 4096);
  assert(lstatDirectory.st_dev);
  assert(lstatDirectory.st_nlink);
  assert(lstatDirectory.st_uid == 0);
  assert(lstatDirectory.st_gid == 0);
#ifdef WASMFS
  assert(lstatDirectory.st_rdev);
  // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
  assert(lstatDirectory.st_blocks == 8);
  assert(lstatDirectory.st_atim.tv_sec == 0);
  assert(lstatDirectory.st_mtim.tv_sec == 0);
  assert(lstatDirectory.st_ctim.tv_sec == 0);
#else
  assert(!lstatDirectory.st_rdev);
  assert(lstatDirectory.st_blocks == 1);
#endif
  assert(lstatDirectory.st_blksize == 4096);

  return 0;
}
