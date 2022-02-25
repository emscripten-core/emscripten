/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "get_backend.h"

int main() {
  int err = wasmfs_create_directory("/root", 0777, get_backend());
  assert(err == 0);
  int fd = open("/root/test", O_CREAT, S_IRUSR | S_IWUSR);
  const char* msg = "Success\n";

  errno = 0;
  write(fd, msg, strlen(msg));
  assert(errno == 0);

  EM_ASM({
    var output = FS.readFile("/root/test", {encoding : 'utf8'});
    out(output);
  });

  EM_ASM({
    try {
      var output = FS.readFile("/root/no-exist", {encoding : 'utf8'});
    } catch (err) {
    }
  });

  EM_ASM({
    try {
      var output = FS.readFile("", {encoding : 'utf8'});
    } catch (err) {
    }
  });
}
