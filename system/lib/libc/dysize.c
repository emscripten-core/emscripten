/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <sys/time.h>

int dysize(int year) {
  int leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
  return leap ? 366 : 365;
}
