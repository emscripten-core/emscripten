/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>

void vary(const char *s, ...) {
  va_list v;
  va_start(v, s);
  char d[20];
  vsnprintf(d, 20, s, v);
  puts(d);

  // Try it with copying
  va_list tempva;
  va_copy(tempva, v);
  vsnprintf(d, 20, s, tempva);
  puts(d);

  va_end(v);
}

void vary2(char color, const char *s, ...) {
  va_list v;
  va_start(v, s);
  char d[21];
  d[0] = color;
  vsnprintf(d + 1, 20, s, v);
  puts(d);
  va_end(v);
}

void varargs_listoffsets_list_evaluate(int count, va_list ap, int vaIteration) {
  while (count > 0) {
    const char *string = va_arg(ap, const char *);
    printf("%s", string);
    count--;
  }
  printf("\n");
}

void varags_listoffsets_list_copy(int count, va_list ap, int iteration) {
  va_list ap_copy;
  va_copy(ap_copy, ap);
  varargs_listoffsets_list_evaluate(count, ap_copy, iteration);
  va_end(ap_copy);
}

void varargs_listoffsets_args(int type, int count, ...) {
  va_list ap;
  va_start(ap, count);

  // evaluate a copied list
  varags_listoffsets_list_copy(count, ap, 1);
  varags_listoffsets_list_copy(count, ap, 2);
  varags_listoffsets_list_copy(count, ap, 3);
  varags_listoffsets_list_copy(count, ap, 4);
}

void varargs_listoffsets_main() {
  varargs_listoffsets_args(0, 5, "abc", "def", "ghi", "jkl", "mno", "<BAD+0>",
                           "<BAD+1>", "<BAD+2>", "<BAD+3>", "<BAD+4>",
                           "<BAD+5>", "<BAD+6>", "<BAD+7>", "<BAD+8>",
                           "<BAD+9>", "<BAD+10>", "<BAD+11>", "<BAD+12>",
                           "<BAD+13>", "<BAD+14>", "<BAD+15>", "<BAD+16>");
}

#define GETMAX(pref, type)              \
  type getMax##pref(int num, ...) {     \
    va_list vv;                         \
    va_start(vv, num);                  \
    type maxx = va_arg(vv, type);       \
    for (int i = 1; i < num; i++) {     \
      type curr = va_arg(vv, type);     \
      maxx = curr > maxx ? curr : maxx; \
    }                                   \
    va_end(vv);                         \
    return maxx;                        \
  }
GETMAX(i, int);
GETMAX(D, double);

int main(int argc, char **argv) {
  vary("*cheez: %d+%d*", 0,
       24);          // Also tests that '0' is not special as an array ender
  vary("*albeit*");  // Should not fail with no var args in vararg function
  vary2('Q', "%d*", 85);

  int maxxi = getMaxi(6, 2, 5, 21, 4, -10, 19);
  printf("maxxi:%d*\n", maxxi);
  double maxxD = getMaxD(6, (double)2.1, (double)5.1, (double)22.1, (double)4.1,
                         (double)-10.1, (double)19.1, (double)2);
  printf("maxxD:%.2f*\n", (float)maxxD);

  // And, as a function pointer
  void (*vfp)(const char * s, ...) = argc == 1211 ? NULL : vary;
  vfp("*vfp:%d,%d*", 22, 199);

  // ensure lists work properly when copied, reinited etc.
  varargs_listoffsets_main();

  return 0;
}
