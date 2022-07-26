/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

int main(int argc, char **argv) {
  int k;
  #pragma clang loop unroll(disable)
  for (int i = 0; i < 100; ++i) {
    k = 0x7fffffff;
    k += argc;
  }
  k = 0x7fffffff; k += argc;
  k = 0x7fffffff; k += argc;
  k = 0x7fffffff; k += argc;
  k = 0x7fffffff; k += argc;
  return 0;
}
