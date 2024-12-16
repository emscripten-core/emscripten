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
#include <assert.h>

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
  assert(fd != -1);
  // Check it exists
  assert(access(filename, F_OK) == 0);
  // Delete the file
  assert(unlinkat(AT_FDCWD, filename, 0) == 0);
  // Check that it doesn't exist
  assert(access(filename, F_OK) == -1);
  // Check that we can still write to it
  assert(write(fd, "hello", 5) == 5);
  // And seek in it.
  assert(lseek(fd, 0, SEEK_SET) == 0);
  // And read from it.
  char buf[6] = {0};
  auto r = read(fd, buf, 5);
  assert(r==5);
  assert(strcmp("hello", buf) == 0);
  assert(close(fd)==0);

  // Create a directory
  assert(mkdir(dirname, 0700) == 0);
  // Open the directory
  DIR* d = opendir(dirname);
  assert(d != NULL);
  // Delete the directory
  assert(unlinkat(AT_FDCWD, dirname, AT_REMOVEDIR) == 0);
  // Check that it doesn't exist
  assert(access(dirname, F_OK) == -1);

  // The rest of this test does not yet pass with the node backend!
#ifndef WASMFS_NODE_BACKEND

  // Check that we can still read the directory, but that it is empty.
  errno = 0;
  assert(readdir(d) == NULL && errno == 0);
  // Check that we *cannot* create a child.
  assert(openat(dirfd(d), filename, O_CREAT | O_WRONLY, S_IRWXU) == -1);
  printf("%s\n", strerror(errno));
#ifdef __EMSCRIPTEN__
  // Linux allows "." and ".." to be accessed on unlinked directories, but this
  // is inconsistent with the result of getdents and would unnecessarily
  // complicate the file system implementation.
  // TODO: Consider supporting "." on unlinked files, if not ".."

  // Check that we cannot still access "."
  assert(openat(dirfd(d), ".", O_DIRECTORY | O_RDONLY) == -1);
#ifdef WASMFS
  // Check that we cannot still access ".." on WasmFS.
  assert(openat(dirfd(d), "..", O_DIRECTORY | O_RDONLY) == -1);
#endif
#else
  // Check that we can still access "." on Linux.
  int self = openat(dirfd(d), ".", O_DIRECTORY | O_RDONLY);
  assert(self != -1);
  close(self);
  // Check that we can still access ".." on Linux.
  int parent = openat(dirfd(d), "..", O_DIRECTORY | O_RDONLY);
  assert(parent != -1);
  close(parent);
#endif

#endif // WASMFS_NODE_BACKEND

  closedir(d);

  printf("ok\n");

  return 0;
}
