/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
typedef long long qint64; /* 64 bit signed */
typedef double qreal;

int main(int argc, char **argv) {
  qreal c = 111;
  qint64 d = -111 + (argc - 1);
  c += d;
  if (c < -1 || c > 1) {
    printf("Failed!\n");
  } else {
    printf("Succeeded!\n");
  }
};
