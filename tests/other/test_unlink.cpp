// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef WASMFS
#include "../wasmfs/get_backend.h"
#include <emscripten/wasmfs.h>
#endif

int main() {
  const char *filename = "test.dat";
  const char *dirname = "test";

#ifdef WASMFS
  if (wasmfs_create_directory("root", 0777, get_backend()) != 0) {
    return 1;
  }
  if (chdir("root") != 0) {
    return 1;
  }
#endif

  // Create a file
  int fd = open(filename, O_RDWR | O_CREAT, 0777);
  if (fd == -1) {
    return 1;
  }
  // Check it exists
  if (access(filename, F_OK) != 0) {
    return 1;
  }
  // Delete the file
  if (unlinkat(AT_FDCWD, filename, 0)) {
    return 1;
  }
  // Check that it doesn't exist
  if (access(filename, F_OK) != -1) {
    return 1;
  }
  // Check that we can still write to it
  if (write(fd, "hello", 5) != 5) {
    return 1;
  }
  // And seek in it.
  if (lseek(fd, 0, SEEK_SET) != 0) {
    return 1;
  }
  // And read from it.
  char buf[6] = {0};
  auto r = read(fd, buf, 5);
  if (r != 5) {
    return 1;
  }
  if (strcmp("hello", buf) != 0) {
    return 1;
  }
  if (close(fd)) {
    return 1;
  }

  // Create a directory
  if (mkdir(dirname, 0700) != 0) {
    return 1;
  }
  // Open the directory
  DIR* d = opendir(dirname);
  if (d == NULL) {
    return 1;
  }
  // Delete the directory
  if (unlinkat(AT_FDCWD, dirname, AT_REMOVEDIR)) {
    return 1;
  }
  // Check that it doesn't exist
  if (access(dirname, F_OK) != -1) {
    return 1;
  }

  // The rest of this test does not yet pass with the node backend!
#ifndef WASMFS_NODE_BACKEND

  // Check that we can still read the directory, but that it is empty.
  errno = 0;
  if (readdir(d) != NULL || errno != 0) {
    return 1;
  }
  // Check that we *cannot* create a child.
  if (openat(dirfd(d), filename, O_CREAT | O_WRONLY, S_IRWXU) != -1) {
    return 1;
  }
  printf("%s\n", strerror(errno));
#ifdef __EMSCRIPTEN__
  // Linux allows "." and ".." to be accessed on unlinked directories, but this
  // is inconsistent with the result of getdents and would unnecessarily
  // complicate the file system implementation.
  // TODO: Consider supporting "." on unlinked files, if not ".."

  // Check that we cannot still access "."
  if (openat(dirfd(d), ".", O_DIRECTORY | O_RDONLY) != -1) {
    return 1;
  }
#ifdef WASMFS
  // Check that we cannot still access ".." on WasmFS.
  if (openat(dirfd(d), "..", O_DIRECTORY | O_RDONLY) != -1) {
    return 1;
  }
#endif
#else
  // Check that we can still access "." on Linux.
  int self = openat(dirfd(d), ".", O_DIRECTORY | O_RDONLY);
  if (self == -1) {
    return 1;
  }
  close(self);
  // Check that we can still access ".." on Linux.
  int parent = openat(dirfd(d), "..", O_DIRECTORY | O_RDONLY);
  if (parent == -1) {
    return 1;
  }
  close(parent);
#endif

#endif // WASMFS_NODE_BACKEND

  closedir(d);

  printf("ok\n");

  return 0;
}
