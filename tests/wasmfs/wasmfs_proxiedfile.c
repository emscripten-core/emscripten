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

void write_and_read(const char* msg, int fd) {
  char buf[200] = {};
  errno = 0;
  write(fd, msg, strlen(msg));
  assert(errno == 0);
  errno = 0;
  lseek(fd, 0, SEEK_SET);
  assert(errno == 0);
  read(fd, buf, sizeof(buf));
  assert(errno == 0);
  printf("%s\n", buf);
}

int main() {

  // Create proxied proxied backend using composition.
  backend_t JSBackend = wasmfs_create_js_file_backend();
  backend_t proxiedBackend = wasmfs_create_proxied_backend(JSBackend);

  // Create a new proxied file under root.
  int fd = wasmfs_create_file("/testfile", 0777, proxiedBackend);

  // Try writing to and reading from the file.
  const char* msg = "Test with a new proxied file\n";
  write_and_read(msg, fd);

  // Verify that the size of the proxied file is the same as the written buffer.
  struct stat file;
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == strlen(msg));

  // Try appending to the end of the proxied file.
  write_and_read(msg, fd);

  // Verify that the size of the proxied file has increased.
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == strlen(msg) * 2);

  // Check that the proxied file has the correct backend type.
  assert(wasmfs_get_backend_by_fd(fd) == proxiedBackend);

  close(fd);

  assert(unlink("/testfile") != -1);

  // Try creating a new proxied backend directory under root.
  assert(wasmfs_create_directory("/test-dir", 0777, proxiedBackend) != -1);

  // Try to create a new proxied file under this new directory.
  int fd2 = open("/test-dir/proxiedfile", O_RDWR | O_CREAT, 0777);

  // Try writing to and reading from the file.
  const char* msg2 =
    "Test with a proxied file created under a proxied directory\n";

  write_and_read(msg2, fd2);

  // Check that the proxied file has the correct backend type.
  assert(wasmfs_get_backend_by_fd(fd2) == proxiedBackend);

  // Try creating an in-memory file under a proxied backend directory.
  backend_t memoryBackend = wasmfs_get_backend_by_path("/");

  int fd3 = wasmfs_create_file("/test-dir/inmemoryfile", 0777, memoryBackend);

  const char* msg3 =
    "Test with an in-memory file created under a proxied directory\n";
  write_and_read(msg3, fd3);

  return 0;
}
