// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main() {
  const char *home = getenv("HOME");
  for (unsigned int i = 0; i < 5; ++i) {
    const char *curr = getenv("HOME");
    assert(curr == home);
  }
  printf("ok\n");
}

