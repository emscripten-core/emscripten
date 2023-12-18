/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// embuilder build libcompiler_rt --force
// emcc -nostartfiles -sASSERTIONS=0 -sSUPPORT_ERRNO=0 \
//   -sEXPORTED_FUNCTIONS=_foo -Wl,--no-entry -g -o foo.wasm no_main.c
// llvm-dwarfdump -debug-info -debug-line --recurse-depth=0 foo.wasm \
//   >foo.wasm.dump
int foo() { return 42; }
