/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <typeinfo>

class Class {};

int main() {
  const Class* dp = dynamic_cast<const Class*>(&typeid(Class));
  // should return dp == NULL,
  printf("pointer: %p\n", dp);
}
