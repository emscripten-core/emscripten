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

struct IUB iub[] = {{'a', 0.27, 5}, {'c', 0.15, 4}, {'g', 0.12, 3}, {'t', 0.27, 2}, };

const unsigned char faceedgesidx[6][4] = {{4, 5, 8, 10},
                                          {6, 7, 9, 11},
                                          {0, 2, 8, 9},
                                          {1, 3, 10, 11},
                                          {0, 1, 4, 6},
                                          {2, 3, 5, 7}, };

int main(int argc, const char *argv[]) {
  printf("*%d,%d,%d,%d*\n", iub[0].c, (int)(iub[1].p * 100), iub[2].pi,
         faceedgesidx[3][2]);
  return 0;
}
