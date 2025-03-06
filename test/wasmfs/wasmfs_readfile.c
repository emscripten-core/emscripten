/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/em_asm.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  int err = mkdir("root", 0777);
  assert(err == 0);
  int fd = open("root/test", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  const char* msg = "Success\n";

  errno = 0;
  write(fd, msg, strlen(msg));
  assert(errno == 0);
  err = close(fd);
  assert(err == 0);

  EM_ASM({
    var output = FS.readFile("root/test");
    out(UTF8ArrayToString(output));
    out("Length: " + output.byteLength);
    var err = FS.unlink("root/test");
    out("FS.unlink: " + err);
  });

  EM_ASM({
    try {
      var output = FS.readFile("root/no-exist", {encoding : 'utf8'});
    } catch (err) {
      return;
    }
    assert(false);
  });

  EM_ASM({
    try {
      // Already unlinked above, file should not exist anymore
      var output = FS.readFile("root/test", {encoding : 'utf8'});
    } catch (err) {
      return;
    }
    assert(false);
  });
}
