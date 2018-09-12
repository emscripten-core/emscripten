/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

int main() {
  int result = EM_ASM_INT({
    return !!Module['memoryInitializerRequest'];
  });
  printf("memory init request: %d\n", result);
  REPORT_RESULT(result);
  return 0;
}

