/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main() {
  char pYear[16], pMonth[16], pDay[16], pDate[64];
  printf("%d\n", sscanf("Nov 19 2012", "%s%s%s", pMonth, pDay, pYear));
  printf("day %s, month %s, year %s \n", pDay, pMonth, pYear);
  return (0);
}
