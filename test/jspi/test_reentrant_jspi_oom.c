// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

// main is a promising export, so entering it needs a fiber stack, which the
// heap cannot provide here.
int main() {
  printf("entered\n");
  return 0;
}
