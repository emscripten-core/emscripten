/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void create_file(const char* path, const char* contents) {
  int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  assert(fd >= 0);
  assert(write(fd, contents, strlen(contents)) == strlen(contents));
  assert(close(fd) == 0);
}

void check_file(const char* path, const char* contents) {
  char buf[64] = {0};
  int fd = open(path, O_RDONLY);
  assert(fd >= 0);
  assert(read(fd, buf, sizeof(buf) - 1) == strlen(contents));
  assert(strcmp(buf, contents) == 0);
  assert(close(fd) == 0);
}

int main() {
  // A renamed file must not refer to its old name, even once something else
  // takes that name.
  create_file("a", "file a");
  assert(rename("a", "b") == 0);
  create_file("a", "new a");
  check_file("b", "file a");
  check_file("a", "new a");

  // Everything already looked up below a renamed directory moves with it.
  assert(mkdir("d", 0777) == 0);
  assert(mkdir("d/sub", 0777) == 0);
  create_file("d/f", "f");
  create_file("d/sub/g", "g");
  assert(symlink("f", "d/link") == 0);
  int f = open("d/f", O_RDWR);
  assert(f >= 0);
  struct stat st;
  assert(stat("d/sub/g", &st) == 0);
  assert(lstat("d/link", &st) == 0);

  assert(rename("d", "e") == 0);

  assert(pwrite(f, "F", 1, 0) == 1);
  assert(close(f) == 0);
  check_file("e/f", "F");
  check_file("e/link", "F");
  check_file("e/sub/g", "g");
  create_file("e/sub/new", "new");
  check_file("e/sub/new", "new");
  assert(access("d", F_OK) == -1 && errno == ENOENT);

  // Moving a symlink.
  assert(rename("e/link", "e/sub/link") == 0);
  char target[16] = {0};
  assert(readlink("e/sub/link", target, sizeof(target) - 1) == 1);
  assert(strcmp(target, "f") == 0);

#if defined(NODEFS) || defined(NODERAWFS)
  // As on Linux, a mount point can't be renamed.
  assert(mkdir("m", 0777) == 0);
  assert(wasmfs_create_directory(
           "m/mnt", 0777, wasmfs_create_memory_backend()) == 0);
  assert(rename("m/mnt", "m/mnt2") == -1);
  assert(errno == EBUSY);
#endif

  puts("done");
  return 0;
}
