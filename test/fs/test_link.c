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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void create_file(const char* path, const char* data) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  assert(fd >= 0);
  assert(write(fd, data, strlen(data)) == (ssize_t)strlen(data));
  close(fd);
}

int main() {
  create_file("foo", "hello");

#if defined(MEMFS) || defined(NODEFS)
  // MEMFS and NODEFS provide no `link` node op, so hardlinks report EMLINK.
  // NODEFS intentionally omits it: unlike NODERAWFS it confines access to a
  // mount root, but a real host hardlink cannot be guaranteed to stay within
  // that root (host symlinks in intermediate path components escape it).
  assert(link("foo", "bar") == -1);
  assert(errno == EMLINK);
#else
  // NODERAWFS: real hardlinks backed by the host filesystem.
  struct stat st;
  assert(link("foo", "bar") == 0);
  assert(stat("foo", &st) == 0);
  assert(st.st_nlink == 2);
  assert(stat("bar", &st) == 0);
  assert(st.st_nlink == 2);

  char buf[7] = {0};
  int fd = open("bar", O_RDONLY);
  assert(fd >= 0);
  assert(read(fd, buf, 5) == 5);
  assert(strcmp(buf, "hello") == 0);
  close(fd);

  // Appending through one link is visible through the other: same inode.
  fd = open("foo", O_WRONLY | O_APPEND);
  assert(fd >= 0);
  assert(write(fd, "!", 1) == 1);
  close(fd);
  assert(stat("bar", &st) == 0);
  assert(st.st_size == 6);

  // linkat with AT_SYMLINK_FOLLOW dereferences a symlink to its target, so the
  // new link shares foo's inode (nlink becomes 3) rather than the symlink's.
  assert(symlink("foo", "sym") == 0);
  assert(linkat(AT_FDCWD, "sym", AT_FDCWD, "baz", AT_SYMLINK_FOLLOW) == 0);
  assert(lstat("baz", &st) == 0);
  assert(!S_ISLNK(st.st_mode));
  assert(st.st_nlink == 3);
  assert(st.st_size == 6);
#endif

  printf("done\n");
  return 0;
}
