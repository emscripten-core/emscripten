/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static backend_t make_js_file_backend(void* arg) {
  return wasmfs_create_js_file_backend();
}

int main() {
  backend_t backend = wasmfs_create_fetch_backend("TODO/url");

  // Create a file in that backend.
  int fd = wasmfs_create_file("/testfile", 0777, backend);

  // Get the size of the file. This will cause a transparent fetch of the data,
  // after which the size is > 0
  struct stat file;
  assert(fstat(fd, &file) != -1);
  printf("file size: %lld\n", file.st_size);
  assert(file.st_size > 0);

  // Create a second file.
  int fd2 = wasmfs_create_file("/testfile2", 0777, backend);

  // Read the data from the file. As before, when we need the data it is
  // transparently fetched for us.
  char buf[file.st_size];
  int bytes_read = read(fd, buf, file.st_size);
  printf("read size: %d\n", bytes_read);
  assert(errno == 0);
  buf[bytes_read] = 0;
  printf("%s\n", buf);
  assert(strcmp(buf, "hello, fetch") == 0);

  return 0;
}
