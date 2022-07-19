/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <time.h>
#include <stdio.h>

void
print_time(time_t time)
{
  char gmbuf[32], locbuf[32];
  const char fmt[] = "%Y-%m-%d %H:%M:%S";
  struct tm *gm;
  struct tm *loc;

  gm = gmtime(&time);
  assert(strftime(gmbuf, sizeof(gmbuf) - 1, fmt, gm) > 0);
  loc = localtime(&time);
  assert(strftime(locbuf, sizeof(locbuf) - 1, fmt, loc) > 0);
  printf("time: %lld, gmtime: %s, localtime: %s\n", time, gmbuf, locbuf);
}

int main() {
    // test assumes EST+05
    print_time(0);
    print_time(2147483647); // int8_t max, Y2K38
    print_time(2147483648);
    print_time(-2147483648); // int8_t min
    print_time(-2147483649);
    print_time(253402300799);
    print_time(-62135596800 + 5 * 3600);
    puts("success");
}
