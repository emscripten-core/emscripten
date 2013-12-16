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
  printf("*%d,%d,%d*\n", (int)s->strstart, (int)s->match_start,
         (int)(s->strstart - s->match_start));
  sh = s->strstart - s->match_start;
  printf("*%d,%d*\n", sh, sh >> 7);

  s->match_start = (char *)32999;
  s->strstart = (char *)(32780);
  printf("*%d,%d,%d*\n", (int)s->strstart, (int)s->match_start,
         (int)(s->strstart - s->match_start));
  sh = s->strstart - s->match_start;
  printf("*%d,%d*\n", sh, sh >> 7);
}
