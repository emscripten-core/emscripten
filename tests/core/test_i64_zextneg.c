/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  uint8_t byte = 0x80;
  uint16_t two = byte;
  uint32_t four = byte;
  uint64_t eight = byte;

  printf("value: %d,%d,%d,%lld.\n", byte, two, four, eight);

  return 0;
}
