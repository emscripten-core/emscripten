/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This file should no longer be need once we land the llvm-side change to
// swtich to using __main_argc_argv:
// TODO(https://reviews.llvm.org/D75277)

// See https://github.com/CraneStation/wasi-libc/pull/152

int __main_void(void);

__attribute__((weak))
int __original_main(void) {
  return __main_void();
}
