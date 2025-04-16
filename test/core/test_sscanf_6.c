/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
int main() {
  const char *date = "18.07.2013w";
  char c[10];
  memset(c, 0, 10);
  int y, m, d, i;
  i = sscanf(date, "%d.%d.%4d%c", &d, &m, &y, c);
  printf("date: %s; day %2d, month %2d, year %4d, extra: %c, %d\n", date, d, m,
         y, c[0], i);
  i = sscanf(date, "%d.%d.%3c", &d, &m, c);
  printf("date: %s; day %2d, month %2d, year %4d, extra: %s, %d\n", date, d, m,
         y, c, i);

  {
    const char *date = "18.07.2013";
    char c;
    int y, m, d, i;
    if ((i = sscanf(date, "%d.%d.%4d%c", &d, &m, &y, &c)) == 3)
    {
        printf("date: %s; day %2d, month %2d, year %4d \n", date, d, m, y);
    }
    else
    {
        printf("Error in sscanf: actually parsed %d", i);
    }
  }
}

