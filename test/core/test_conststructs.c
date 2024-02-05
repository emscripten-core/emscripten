/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
struct IUB {
  int c;
  double p;
  unsigned int pi;
};

int main(int argc, const char *argv[]) {
  int before = 70;
  struct IUB iub[] = {{'a', 0.3029549426680, 5},
               {'c', 0.15, 4},
               {'g', 0.12, 3},
               {'t', 0.27, 2}, };
  int after = 90;
  printf("*%d,%d,%d,%d,%d,%d*\n", before, iub[0].c, (int)(iub[1].p * 100),
         iub[2].pi, (int)(iub[0].p * 10000), after);
  return 0;
}
