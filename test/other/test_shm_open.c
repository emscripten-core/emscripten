// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  const char* name = "/test_shm";

  // Invalid name with multiple path segments
  int fd = shm_open("/invalid/name", O_RDWR | O_CREAT, 0666);
  assert(fd == -1);
  assert(errno == EINVAL);

  // Opening non-existent object without O_CREAT fails
  fd = shm_open(name, O_RDWR, 0);
  assert(fd == -1);
  assert(errno == ENOENT);

  // Creating with O_CREAT | O_EXCL succeeds
  fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
  assert(fd >= 0);

  // Creating with O_CREAT | O_EXCL when already exists fails
  int fd_dup = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
  assert(fd_dup == -1);
  assert(errno == EEXIST);

  // Write data to the shared memory object
  const char msg[] = "hello shm";
  assert(write(fd, msg, sizeof(msg)) == sizeof(msg));
  assert(close(fd) == 0);

  // Reopen existing object and read data back
  fd = shm_open(name, O_RDONLY, 0);
  assert(fd >= 0);
  char buf[32] = {0};
  assert(read(fd, buf, sizeof(msg)) == sizeof(msg));
  assert(strcmp(buf, msg) == 0);
  assert(close(fd) == 0);

  // Unlink the shared memory object
  assert(shm_unlink(name) == 0);

  // Unlinking again fails
  assert(shm_unlink(name) == -1);
  assert(errno == ENOENT);

  // Reopening after unlink fails
  fd = shm_open(name, O_RDONLY, 0);
  assert(fd == -1);
  assert(errno == ENOENT);

  printf("done\n");
  return 0;
}
