// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <emscripten/stack.h>

// Allocate more than the default 64KB stack per frame so that even when
// static data sits below the stack, the stack pointer jumps past zero and
// traps rather than overwriting the stack cookie.
#define FRAME_SIZE (128 * 1024)

int recurse(int depth, unsigned char* prev) {
  unsigned char buffer[FRAME_SIZE];
  memset(buffer, depth, sizeof(buffer));
  printf("recurse %d sp=%#" PRIxPTR "\n", depth, emscripten_stack_get_current());
  if (prev) {
    prev[0] += depth;
    prev[FRAME_SIZE - 1] += depth;
  }
  if (depth > 0) {
    return recurse(depth - 1, buffer) + buffer[0] + buffer[FRAME_SIZE - 1];
  }
  return buffer[0] + buffer[FRAME_SIZE - 1];
}

int main() {
  return recurse(1000, NULL);
}
