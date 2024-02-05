/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

struct Class {
  static const char *name1() { return "nameA"; }
  const char *name2() { return "nameB"; }
};

int main() {
  printf("*%s,%s*\n", Class::name1(), Class().name2());
  return 0;
}
