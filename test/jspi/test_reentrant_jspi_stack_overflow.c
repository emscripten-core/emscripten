// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

// The frame is larger than the fiber stack; the overflow is caught when the
// fiber leaves its stack at the suspension.
int main() {
  volatile char big[17 * 1024];
  for (int i = 0; i < (int)sizeof(big); i++) {
    big[i] = (char)i;
  }
  emscripten_sleep(1);
  printf("resumed %d\n", big[0]);
  return 0;
}
