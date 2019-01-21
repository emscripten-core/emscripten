// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  int sum = EM_ASM_INT({
     return $0 + $2;
  }, 0, 1, 2);
  printf("0+2=%d\n", sum);
  return 0;
}
