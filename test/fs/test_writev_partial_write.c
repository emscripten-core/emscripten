/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <emscripten.h>

EM_JS_DEPS(main, "$ERRNO_CODES");

void setup(void) {
 EM_ASM(
    var device = FS.makedev(80, 0);
    FS.registerDevice(device, {
      write: function(stream, buffer, offset, length, pos) {
        if (length === 0) {
          return 0;
        }
        // Only do a partial write of one byte each time
        out('TO DEVICE: ' + JSON.stringify(String.fromCharCode(buffer[offset])));
        return 1;
      }
    });
    FS.mkdev('/device', device);
  );
}

// Run the test with writev directly
void test_writev_direct(void) {
  int fd = open("/device", O_WRONLY);
  assert(fd);
  struct iovec iovs[] = {{.iov_base = "ABC", .iov_len = 3},
                         {.iov_base = "XYZ", .iov_len = 3}};
  struct iovec* iov = iovs;

  size_t rem = iov[0].iov_len + iov[1].iov_len;
  int iovcnt = 2;
  ssize_t cnt;
  for (;;) {
    cnt = writev(fd, iov, iovcnt);
    assert(cnt >= 0);
    if (cnt == rem) {
      // All data written
      break;
    }
    rem -= cnt;
    if (cnt > iov[0].iov_len) {
      cnt -= iov[0].iov_len;
      iov++;
      iovcnt--;
    }
    iov[0].iov_base = (char*)iov[0].iov_base + cnt;
    iov[0].iov_len -= cnt;
  }

  close(fd);
}

// Run the test using stdio, this test is dependent on specific buffering used
// and is included here as this code most closely matches the original bug
// report
void test_via_stdio(void) {
  FILE* f = fopen("/device", "w");
  assert(f);
  // Use line buffering. The bug is exposed with line buffering because with
  // line buffering two entries in __stdio_write's iovs are used.
  setvbuf(f, NULL, _IOLBF, 0);
  fputs("abc", f);
  fputs("\n", f);
  fflush(f);
  fclose(f);
}

int main() {
  setup();
  test_writev_direct();
  test_via_stdio();
  printf("done\n");
  return 0;
}
