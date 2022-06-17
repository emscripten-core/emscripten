/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <features.h>

static int _main_argc;
static char** _main_argv;

weak int __main_argc_argv(int argc, char *argv[]);

weak int __main_void(void) {
  if (__main_argc_argv) {
    return __main_argc_argv(_main_argc, _main_argv);
  }
  return 0;
}

EMSCRIPTEN_KEEPALIVE int _emscripten_start(int argc, char** argv) {
  _main_argc = argc;
  _main_argv = argv;
  // Will either call user's __main_void or weak version above.
  return __main_void();
}
