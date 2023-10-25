/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <emscripten/console.h>

void *malloc(size_t size) { return (void *)123; }
int main() {
  void *x = malloc(10);
  emscripten_outf("got %p", x);
  free(0);
  emscripten_outf("freed a fake");
  return 1;
}
