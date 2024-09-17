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

void write_and_read(const char* msg, int fd, const char* expected) {
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
  assert(strcmp(buf, expected) == 0);
}

static backend_t make_js_file_backend(void* arg) {
  return wasmfs_create_js_file_backend();
}

int main() {
  backend_t backend = make_js_file_backend(NULL);

  // Create a new backend file under root.
  int fd = wasmfs_create_file("/testfile", 0777, backend);

  // Ensure that the size of the backend file is zero.
  struct stat file;
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == 0);

  // Try writing to and reading from the file.
  const char* msg = "Test with a new backend file\n";
  write_and_read(msg, fd, msg);

  // Verify that the size of the backend File is the same as the written buffer.
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == strlen(msg));

  // Try seeking to the beginning of the file and overwriting its contents.
  assert(lseek(fd, 0, SEEK_SET) != -1);
  write_and_read(msg, fd, msg);

  // Try appending to the end of the backend File.
  write_and_read(
    msg, fd, "Test with a new backend file\nTest with a new backend file\n");

  // Verify that the size of the backend File has increased.
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == strlen(msg) * 2);

  close(fd);

  assert(unlink("/testfile") != -1);

  // Try creating a new backend directory under root.
  assert(wasmfs_create_directory("/test-dir", 0777, backend) != -1);

  // Try to create a new backend file under this new directory.
  int fd2 = open("/test-dir/backendfile", O_RDWR | O_CREAT, 0777);

  // Try writing to and reading from the file.
  const char* msg2 =
    "Test with a backend file created under a backend directory\n";

  write_and_read(msg2, fd2, msg2);

  // Check that the backend file has the correct backend type.
  assert(wasmfs_get_backend_by_fd(fd2) == backend);

  // Try creating an in-memory file under a backend directory.
  backend_t memoryBackend = wasmfs_get_backend_by_path("/");

  int fd3 = wasmfs_create_file("/test-dir/inmemoryfile", 0777, memoryBackend);

  const char* msg3 =
    "Test with an in-memory file created under a backend directory\n";
  write_and_read(msg3, fd3, msg3);

  // Create a new backend file under root.
  int fd4 = wasmfs_create_file("/testfile2", 0777, backend);

  const char* msg4 = "Test with a backend file created under root\n";

  // Try setting the file offset to an arbitrary value.
  assert(lseek(fd4, 10, SEEK_SET) != -1);

  char buf[200] = {};
  errno = 0;
  write(fd4, msg4, strlen(msg4));
  assert(errno == 0);
  errno = 0;
  lseek(fd4, 0, SEEK_SET);
  assert(errno == 0);
  int bytesRead = read(fd4, buf, sizeof(buf));
  for (ssize_t i = 0; i < bytesRead; i++) {
    if (buf[i] == 0) {
      printf("\\0");
    } else {
      printf("%c", buf[i]);
    }
  }
  printf("\n");

  // Check the size.
  int result = lseek(fd4, 0, SEEK_END);
  assert(result != -1);
  printf("size: %lld\n", lseek(fd4, 0, SEEK_CUR));

  // Resize.
  result = ftruncate(fd4, 42);
  assert(!result);
  result = lseek(fd4, 0, SEEK_END);
  assert(result != -1);
  printf("resize: %lld\n", lseek(fd4, 0, SEEK_CUR));

  // Check the contents remain after the resize (but are a tad shorter).
  lseek(fd4, 0, SEEK_SET);
  assert(errno == 0);
  bytesRead = read(fd4, buf, sizeof(buf));
  for (ssize_t i = 0; i < bytesRead; i++) {
    if (buf[i] == 0) {
      printf("\\0");
    } else {
      printf("%c", buf[i]);
    }
  }
  printf("\n");

  puts("\ndone.");
  return 0;
}
