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

int main(int argc, char *argv[]);

__attribute__((__weak__))
int __main_argc_argv(int argc, char *argv[]) {
  return main(argc, argv);
}
