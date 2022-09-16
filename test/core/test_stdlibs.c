/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

void clean()
{
  printf("*cleaned*\n");
}

int comparer(const void *a, const void *b) {
  int aa = *((int*)a);
  int bb = *((int*)b);
  return aa - bb;
}

int main() {
  // timeofday
  struct timeval t;
  gettimeofday(&t, NULL);
  printf("*%d,%d\n", (int)t.tv_sec, (int)t.tv_usec); // should not crash

  // atexit
  atexit(clean);

  // qsort
  int values[6] = { 3, 2, 5, 1, 5, 6 };
  qsort(values, 5, sizeof(int), comparer);
  printf("*%d,%d,%d,%d,%d,%d*\n", values[0], values[1], values[2], values[3], values[4], values[5]);

  printf("*stdin==0:%d*\n", stdin == 0); // check that external values are at least not NULL
  printf("*%%*\n");
  printf("*%.1ld*\n", 5l);

  printf("*%.1f*\n", strtod("66", NULL)); // checks dependency system, as our strtod needs _isspace etc.

  printf("*%ld*\n", strtol("10", NULL, 0));
  printf("*%ld*\n", strtol("0", NULL, 0));
  printf("*%ld*\n", strtol("-10", NULL, 0));
  printf("*%ld*\n", strtol("12", NULL, 16));

  printf("*%lu*\n", strtoul("10", NULL, 0));
  printf("*%lu*\n", strtoul("0", NULL, 0));
  printf("*%lu*\n", strtoul("-10", NULL, 0));

  printf("*%llu*\n", strtoull("10", NULL, 0));
  printf("*%llu*\n", strtoull("0", NULL, 0));
  printf("*%llu*\n", strtoull("-10", NULL, 0));

  free(malloc(0));
  printf("*malloc(0) does not fail horribly (spec allows 0 or non-zero)*\n");

  printf("tolower_l: %c\n", tolower_l('A', 0));

  return 0;
}
