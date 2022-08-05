/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <unistd.h>

int main() {
  write(STDOUT_FILENO, "a", 1);
  write(STDOUT_FILENO, "b", 1);
  write(STDOUT_FILENO, "c", 1);
  write(STDOUT_FILENO, "end\n", 4);

  write(STDOUT_FILENO, "d", 1);
  fsync(STDOUT_FILENO);
  write(STDOUT_FILENO, "e", 1);
  fsync(STDOUT_FILENO);
  write(STDOUT_FILENO, "f", 1);
  fsync(STDOUT_FILENO);
  return 0;
}
