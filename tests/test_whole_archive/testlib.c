/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Library containing only single static constructor function.
// This is used to verify that the -Wl,-whole-archive works as expected

extern int foo;

void library_ctor() __attribute__ ((constructor));
void library_ctor() {
  foo = 42;
}
