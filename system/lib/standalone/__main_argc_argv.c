/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// New compilers define `__main_argc_argv`. If that doesn't exist, we
// may get called here. Old compilers define `main` expecting an
// argv/argc, so call that.
// TODO: Remove this layer when we no longer have to support old compilers.
int __legacy_main(int argc, char *argv[]) __asm("main");

__attribute__((__weak__))
int __main_argc_argv(int argc, char *argv[]) {
  return __legacy_main(argc, argv);
}
