/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
struct S {
  char *match_start;
  char *strstart;
};
int main() {
  struct S _s;
  struct S *s = &_s;
  unsigned short int sh;

  s->match_start = (char *)32522;
  s->strstart = (char *)(32780);
  printf("*%ld,%ld,%ld*\n", (long)s->strstart, (long)s->match_start,
         (long)(s->strstart - s->match_start));
  sh = s->strstart - s->match_start;
  printf("*%d,%d*\n", sh, sh >> 7);

  s->match_start = (char *)32999;
  s->strstart = (char *)(32780);
  printf("*%ld,%ld,%ld*\n", (long)s->strstart, (long)s->match_start,
         (long)(s->strstart - s->match_start));
  sh = s->strstart - s->match_start;
  printf("*%d,%d*\n", sh, sh >> 7);
}
