/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

static void cleanExit() {
  puts("3");
}

int main(int argc, char **argv) {
  atexit(cleanExit);
  puts("1");
  raise(atoi(argv[1]));
  puts("2");
  return 0;
}
