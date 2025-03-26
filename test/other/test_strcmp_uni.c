/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
int main() {
#define TEST(func)                                  \
  {                                                 \
    char *word = (char*)"WORD";                     \
    char wordEntry[2] = {-61, -126}; /* "Â"; */    \
    int cmp = func(word, wordEntry, 2);             \
    if (cmp < 0) cmp = -1;                          \
    else if (cmp > 0) cmp = 1;                      \
    printf("Compare value " #func " is %d\n", cmp); \
  }
  TEST(strncmp);
  TEST(strncasecmp);
  TEST(memcmp);
}
