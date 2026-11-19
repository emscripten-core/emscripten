/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Regression test: writev must gather all iovecs into a single FS.write so a
// stream socket send is not fragmented into one segment per iovec.

#include <assert.h>
#include <emscripten/emscripten.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

int main() {
  EM_ASM({
    var dev = FS.makedev(64, 0);
    FS.registerDevice(dev, {
      write: (stream, buffer, offset, length, pos) => {
        // A single gather-write for the whole payload, exactly once.
        assert(length == 8, 'expected a single 8-byte gather write');
        var s = String.fromCharCode.apply(null, buffer.subarray(offset, offset + length));
        assert(s == 'a=1\nb=2\n', 'unexpected gather-write payload: ' + s);
        return length;
      }
    });
    FS.mkdev('/wdev', dev);
  });

  int fd = open("/wdev", O_WRONLY);
  assert(fd >= 0);

  char buf0[] = "a=1\n";
  char buf1[] = "b=2\n";
  struct iovec iov[] = {{.iov_base = buf0, .iov_len = 4},
                        {.iov_base = buf1, .iov_len = 4}};
  ssize_t nwritten = writev(fd, iov, 2);
  assert(nwritten == 8);
  close(fd);

  printf("done\n");
  return 0;
}
