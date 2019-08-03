// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

void log(const char* str) {
  EM_ASM({ console.log("log:", UTF8ToString($0)) }, str);
}

void error(const char* str) {
  log(str);
  exit(1);
}

int main() {
  EM_ASM({
    // A silly example of implementing AsyncFSImpl hooks. There are no
    // actual async operations here, we just call the callback immediately,
    // but in real-world code you could wait on a Promise, etc.
    AsyncFSImpl.open = function(pathname, flags, mode, wakeUp) {
      wakeUp(1);
    };
    AsyncFSImpl.ioctl = function(fd, op, wakeUp) {
      wakeUp(0);
    };
    AsyncFSImpl.readv = function(fd, iovs, wakeUp) {
      var total = 0;
      var index = 0;
      iovs.forEach(function(iov) {
        for (var i = 0; i < iov.len; i++) {
          HEAPU8[iov.ptr + i] = index * index;
          index++;
        }
        total += iov.len;
      });
      wakeUp(total);
    };
    AsyncFSImpl.llseek = function(fd, offset_high, offset_low, result, whence, wakeUp) {
      wakeUp(0);
    };
    AsyncFSImpl.close = function(fd, wakeUp) {
      wakeUp(0);
    };
  });

  log("opening");
  FILE* f = fopen("does_not_matter", "r");
  if (!f) error("open error");

  log("reading");
  const int N = 5;
  char buffer[N];
  int rv = fread(buffer, 1, N, f);
  if (rv != N) error("read error");

  log("checking");
  for (int i = 0; i < N; i++) {
    EM_ASM({ console.log("read:", $0, $1) }, i, buffer[i]);
    if (buffer[i] != i * i) error("data error");
  }

  log("closing");
  rv = fclose(f);
  if (rv) error("close error");

  log("ok.");

  return 0;
}

