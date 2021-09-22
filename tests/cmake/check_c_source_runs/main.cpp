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
  printf("Hello! __cplusplus: %ld\n", __cplusplus);
}
