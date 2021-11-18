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

int main() {

  backend_t JSBackend = create_js_file_backend();

  // Create a new JS file under root.
  int fd = wasmfs_create("/testfile", 0777, JSBackend);

  // Try writing to and reading from the file.
  errno = 0;
  const char* msg = "Test with a new JS file\n";
  write(fd, msg, strlen(msg));
  assert(errno == 0);

  EM_ASM({out(wasmFS$JSMemoryFiles[0])});

  errno = 0;
  lseek(fd, 0, SEEK_SET);
  assert(errno == 0);
  char buf[200] = {};
  read(fd, buf, sizeof(buf));
  assert(errno == 0);
  printf("%s\n", buf);

  close(fd);

  //   Try creating a new JS directory under root.
  int result = wasmfs_mkdir("/test-dir", 0777, JSBackend);

  //   Try to create a new JS file under this new directory.
  int fd2 = open("/test-dir/jsfile", O_RDWR | O_CREAT, 0777);

  // Try writing to and reading from the file.
  errno = 0;
  const char* msg2 = "Test with a JS file created under a JS directory\n";
  write(fd2, msg2, strlen(msg2));
  assert(errno == 0);

  EM_ASM({out(wasmFS$JSMemoryFiles[1])});

  errno = 0;
  lseek(fd2, 0, SEEK_SET);
  assert(errno == 0);
  read(fd2, buf, sizeof(buf));
  assert(errno == 0);
  printf("%s\n", buf);

  return 0;
}
