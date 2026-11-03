/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Regression test: a partial gather-read/write on a non-blocking stream must
// return the bytes already transferred rather than propagating EAGAIN from a
// later iovec.

#include <assert.h>
#include <emscripten/emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

int main() {
  EM_ASM({
    var eagain = $0;

    // Device that satisfies the first read fully, then would-blocks.
    var readCalls = 0;
    var rdev = FS.makedev(64, 0);
    FS.registerDevice(rdev, {
      read: (stream, buffer, offset, length, pos) => {
        if (++readCalls == 1) {
          for (var i = 0; i < length; i++) buffer[offset + i] = 65 + i;
          return length;
        }
        throw new FS.ErrnoError(eagain);
      }
    });
    FS.mkdev('/rdev', rdev);

    // Device that accepts the first write fully, then would-blocks.
    var writeCalls = 0;
    var wdev = FS.makedev(64, 1);
    FS.registerDevice(wdev, {
      write: (stream, buffer, offset, length, pos) => {
        if (++writeCalls == 1) return length;
        throw new FS.ErrnoError(eagain);
      }
    });
    FS.mkdev('/wdev', wdev);
  }, EAGAIN);

  int rfd = open("/rdev", O_RDONLY);
  assert(rfd >= 0);
  char buf0[4] = {0};
  char buf1[4] = {0};
  struct iovec riov[] = {{.iov_base = buf0, .iov_len = 4},
                         {.iov_base = buf1, .iov_len = 4}};
  ssize_t nread = readv(rfd, riov, 2);
  // First iovec filled; second would-block -> partial success, not EAGAIN.
  assert(nread == 4);
  assert(memcmp(buf0, "ABCD", 4) == 0);
  close(rfd);

  int wfd = open("/wdev", O_WRONLY);
  assert(wfd >= 0);
  char out0[4] = "ABCD";
  char out1[4] = "EFGH";
  struct iovec wiov[] = {{.iov_base = out0, .iov_len = 4},
                         {.iov_base = out1, .iov_len = 4}};
  // writev gathers both iovecs into a single write, so the whole 8 bytes go
  // out in one call and the would-block second call never happens.
  ssize_t nwrite = writev(wfd, wiov, 2);
  assert(nwrite == 8);
  close(wfd);

  printf("done\n");
  return 0;
}
