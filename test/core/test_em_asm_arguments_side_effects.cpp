// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  int counter_1 = 3, counter_2 = 4;
  // https://github.com/emscripten-core/emscripten/issues/9030
  printf("counter_1=%d, counter_2=%d\n", counter_1, counter_2);
  int result = EM_ASM_INT({
     return $0 * 10 + $1;
  }, counter_1++, counter_2++);
  printf("counter_1++ * 10 + counter_2++=%d\n", result);
  printf("counter_1=%d, counter_2=%d\n", counter_1, counter_2);
  return 0;
}
