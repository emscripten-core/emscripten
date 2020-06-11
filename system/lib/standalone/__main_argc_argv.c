/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This file should no longer be needed once we land the llvm-side change to
// switch to using __main_argc_argv:
// TODO(https://reviews.llvm.org/D75277)

// See https://github.com/CraneStation/wasi-libc/pull/152

#include <assert.h>

// Main has to be weak here because in emscripten we allow standalone
// applications that don't contain a main at all.
int main(int argc, char *argv[]) __attribute__((weak));

__attribute__((weak))
int __main_argc_argv(int argc, char *argv[]) {
  if (!main)
    return 0;
  return main(argc, argv);
}
