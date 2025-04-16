// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

__attribute__((aligned(16))) volatile char aligned;

int main() {
  assert((((size_t) &aligned) % 16) == 0);
  puts("done!");
  return 0;
}

