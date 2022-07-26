/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

typedef long long int64;

bool compare(int64 val) { return val == -12; }

bool compare2(int64 val) { return val < -12; }

int main(int argc, char* argv[]) {
  printf("*%d,%d,%d,%d,%d,%d*\n", argc, compare(argc - 1 - 12),
         compare(1000 + argc), compare2(argc - 1 - 10), compare2(argc - 1 - 14),
         compare2(argc + 1000));
  return 0;
}
