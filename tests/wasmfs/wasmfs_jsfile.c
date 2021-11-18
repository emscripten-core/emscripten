/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {

  // Create a JS Backend
  backend_t JSBackend = createJSFileBackend();

  // Create a new JS file under root.
  int fd = wasmfs_create("/testfile", 0777, JSBackend);

  // Try writing to and reading from the file.

  return 0;
}