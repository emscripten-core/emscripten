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

  backend_t JSBackend = wasmfs_create_js_file_backend();

  // Create a new JS file under root.
  int fd = wasmfs_create_file("/testfile", 0777, JSBackend);

  // Ensure that the size of the JS file is zero.
  struct stat file;
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == 0);

  // Try writing to and reading from the file.
  const char* msg = "Test with a new JS file\n";
  write_and_read(msg, fd);
  EM_ASM({out(wasmFS$JSMemoryFiles[0])});

  // Verify that the size of the JS File is the same as the written buffer.
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == strlen(msg));

  // Try seeking to the beginning of the file and overwriting its contents.
  assert(lseek(fd, 0, SEEK_SET) != -1);
  write_and_read(msg, fd);
  EM_ASM({out(wasmFS$JSMemoryFiles[0])});

  // Try appending to the end of the JS File.
  write_and_read(msg, fd);

  // Verify that the size of the JS File has increased.
  assert(fstat(fd, &file) != -1);
  assert(file.st_size == strlen(msg) * 2);

  close(fd);

  assert(unlink("/testfile") != -1);

  // Check that the file has been cleaned up in the JS array.
  EM_ASM({out("Expect null: " + wasmFS$JSMemoryFiles[0])});

  // Try creating a new JS directory under root.
  assert(wasmfs_create_directory("/test-dir", 0777, JSBackend) != -1);

  // Try to create a new JS file under this new directory.
  int fd2 = open("/test-dir/jsfile", O_RDWR | O_CREAT, 0777);

  // Try writing to and reading from the file.
  const char* msg2 = "Test with a JS file created under a JS directory\n";

  write_and_read(msg2, fd2);
  // Under first-fit policy, wasmFS$JSMemoryFiles[0] should be populated again.
  EM_ASM({out(wasmFS$JSMemoryFiles[0])});

  // Check that the JS file has the correct backend type.
  assert(wasmfs_get_backend_by_fd(fd2) == JSBackend);

  // Try creating an in-memory file under a JS directory.
  backend_t memoryBackend = wasmfs_get_backend_by_path("/");

  int fd3 = wasmfs_create_file("/test-dir/inmemoryfile", 0777, memoryBackend);

  // Ensure that the JS array size has not changed.
  int size = EM_ASM_INT({ return wasmFS$JSMemoryFiles.length; });
  assert(size == 1);

  const char* msg3 =
    "Test with an in-memory file created under a JS directory\n";
  write_and_read(msg3, fd3);

  // Create a new JS file under root.
  int fd4 = wasmfs_create_file("/testfile2", 0777, JSBackend);

  const char* msg4 = "Test with a JS file created under root\n";

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

  EM_ASM({out(wasmFS$JSMemoryFiles[1])});

  return 0;
}
