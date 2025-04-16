// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten/wire.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
#if __STRICT_ANSI__
  int strict_ansi = 1;
#else
  int strict_ansi = 0;
#endif
  printf("Hello! __STRICT_ANSI__: %d, __cplusplus: %ld\n", strict_ansi, __cplusplus);
}
