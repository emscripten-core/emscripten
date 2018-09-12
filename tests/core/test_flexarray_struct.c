/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  uint16_t length;
  struct {
    int32_t int32;
  } value[];
} Tuple;

int main() {
  Tuple T[10];
  Tuple *t = &T[0];

  t->length = 4;
  t->value->int32 = 100;

  printf("(%d, %d)\n", t->length, t->value->int32);
  return 0;
}
