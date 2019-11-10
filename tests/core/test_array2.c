/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

static const double grid[4][2] = {{-3 / 3., -1 / 3.},
                                  {+1 / 3., -3 / 3.},
                                  {-1 / 3., +3 / 3.},
                                  {+3 / 3., +1 / 3.}};

int main() {
  for (int i = 0; i < 4; i++)
    printf("%d:%.2f,%.2f ", i, grid[i][0], grid[i][1]);
  printf("\n");
  return 0;
}
