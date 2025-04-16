/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "stdio.h"

static const char *colors[] = {"  c black", ". c #001100", "X c #111100"};

int main() {
  unsigned char code;
  char color[32];
  int rcode;
  for (int i = 0; i < 3; i++) {
    rcode = sscanf(colors[i], "%c c %s", &code, color);
    printf("%i, %c, %s\n", rcode, code, color);
  }
}
