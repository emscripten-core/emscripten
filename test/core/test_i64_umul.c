/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <inttypes.h>
#include <stdio.h>

typedef uint32_t UINT32;
typedef uint64_t UINT64;

int main() {
  volatile UINT32 testu32a = 2375724032U;
  UINT32 bigu32 = 0xffffffffU;
  volatile UINT64 testu64a = 14746250828952703000U;

  while ((UINT64)testu32a * (UINT64)bigu32 < testu64a) {
    printf("testu64a is %llu\n", testu64a);
    testu64a /= 2;
  }

  return 0;
}
