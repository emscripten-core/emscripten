/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
  int fd = open("/test", O_CREAT, S_IRGRP);
  const char* msg = "Success\n";

  errno = 0;
  write(fd, msg, strlen(msg));
  EM_ASM({
    var output = FS.readFile("/test", {encoding : 'utf8'});
    out(output);
  });

  EM_ASM({
    try {
      var output = FS.readFile("/no-exist", {encoding : 'utf8'});
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
