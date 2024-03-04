/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <my_port.h>
#include <assert.h>

int main() {
  assert(my_port_fn(99) == 99); // check that we can call a function from my_port.h
  return 0;
}
