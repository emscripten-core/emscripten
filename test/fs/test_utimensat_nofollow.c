/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  int fd = open("target", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  assert(fd >= 0);
  close(fd);
  assert(symlink("target", "link") == 0);

  struct timespec times[2];
  times[0].tv_sec = 111111111; // atime
  times[0].tv_nsec = 0;
  times[1].tv_sec = 222222222; // mtime
  times[1].tv_nsec = 0;

  // AT_SYMLINK_NOFOLLOW sets the symlink's own timestamps rather than the
  // target's.
  assert(utimensat(AT_FDCWD, "link", times, AT_SYMLINK_NOFOLLOW) == 0);

  struct stat ls, ts;
  assert(lstat("link", &ls) == 0); // the symlink itself
  assert(stat("target", &ts) == 0); // the target it points at
  assert(ls.st_mtim.tv_sec == 222222222);
  assert(ts.st_mtim.tv_sec != 222222222); // target left untouched

  printf("done\n");
  return 0;
}
