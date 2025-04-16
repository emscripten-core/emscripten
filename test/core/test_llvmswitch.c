/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>

int switcher(int p) {
  switch (p) {
    case 'a':
    case 'b':
    case 'c':
      return p - 1;
    case -15:
      return p + 1;
  }
  return p;
}

int main(int argc, const char *argv[]) {
  unsigned int x = 0xfffffff1;
  x >>= (argc - 1);  // force it to be unsigned for purpose of checking our
                     // switch comparison in signed/unsigned
  printf("*%d,%d,%d,%d,%d,%d*\n", switcher('a'), switcher('b'), switcher('c'),
         switcher(x), switcher(-15), switcher('e'));
  return 0;
}
