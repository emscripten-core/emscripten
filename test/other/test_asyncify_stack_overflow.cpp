// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main(int argc, char* argv[]) {
  int x = argc;
  int y = x * x;
  int z = y * y;
  x++;
  y++;
  z++;
  emscripten_sleep(1);
  // We should not get here - the unwind will fail as the stack is too small
  printf("We should not get here %d %d %d\n", x, y, z);

  return 0;
}
