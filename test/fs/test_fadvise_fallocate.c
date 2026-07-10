/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  // Regular file: fallocate extends it and fadvise is a successful no-op.
  int fd = open("file", O_RDWR | O_CREAT | O_TRUNC, 0644);
  assert(fd >= 0);
  assert(posix_fallocate(fd, 0, 100) == 0);
  struct stat st;
  assert(fstat(fd, &st) == 0);
  assert(st.st_size == 100);
  assert(posix_fadvise(fd, 0, 0, POSIX_FADV_NORMAL) == 0);
  close(fd);

  // Pipe: non-seekable, so both report ESPIPE (matching Linux). Its fstat also
  // reports a FIFO even under NODERAWFS, where the stream has no host fd.
  int fds[2];
  assert(pipe(fds) == 0);
  assert(fstat(fds[0], &st) == 0);
  assert(S_ISFIFO(st.st_mode));
  assert(posix_fallocate(fds[1], 0, 10) == ESPIPE);
  assert(posix_fadvise(fds[0], 0, 0, POSIX_FADV_NORMAL) == ESPIPE);
  close(fds[0]);
  close(fds[1]);

  printf("done\n");
  return 0;
}
