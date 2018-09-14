/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

//clang --target=wasm32-unknown-unknown-wasm \
// -nostartfiles -nostdlib -Wl,--no-entry,--export=foo -g -o foo.wasm no_main.c
int foo()
{
  return 42;
}
