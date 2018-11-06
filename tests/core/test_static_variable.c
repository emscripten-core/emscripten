/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

struct DATA {
  int value;

  DATA() { value = 0; }
};

DATA& GetData() {
  static DATA data;

  return data;
}

int main() {
  GetData().value = 10;
  printf("value:%i", GetData().value);
}
