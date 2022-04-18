#include <stdio.h>

struct S4   { int x;          }; // size: 4
struct S4_2 { short x, y;     }; // size: 4, but for alignment purposes, 2
struct S6   { short x, y, z;  }; // size: 6
struct S6w  { char x[6];      }; // size: 6 also
struct S6z  { int x; short y; }; // size: 8, since we align to a multiple of the biggest - 4

struct C___  { S6 a, b, c; int later; };
struct Carr  { S6 a[3]; int later; }; // essentially the same, but differently defined
struct C__w  { S6 a; S6w b; S6 c; int later; }; // same size, different struct
struct Cp1_  { int pre; short a; S6 b, c; int later; }; // fillers for a
struct Cp2_  { int a; short pre; S6 b, c; int later; }; // fillers for a (get addr of the other filler)
struct Cint  { S6 a; int  b; S6 c; int later; }; // An int (different size) for b
struct C4__  { S6 a; S4   b; S6 c; int later; }; // Same size as int from before, but a struct
struct C4_2  { S6 a; S4_2 b; S6 c; int later; }; // Same size as int from before, but a struct with max element size 2
struct C__z  { S6 a; S6z  b; S6 c; int later; }; // different size, 8 instead of 6

int main()
{
  #define TEST(struc) \
  { \
    struc *s = 0; \
    printf("*%s: %ld,%ld,%ld,%ld<%zu*\n", #struc, (long)&(s->a), (long)&(s->b), (long)&(s->c), (long)&(s->later), sizeof(struc)); \
  }
  #define TEST_ARR(struc) \
  { \
    struc *s = 0; \
    printf("*%s: %ld,%ld,%ld,%ld<%zu*\n", #struc, (long)&(s->a[0]), (long)&(s->a[1]), (long)&(s->a[2]), (long)&(s->later), sizeof(struc)); \
  }
  printf("sizeofs:%zu,%zu\n", sizeof(S6), sizeof(S6z));
  TEST(C___);
  TEST_ARR(Carr);
  TEST(C__w);
  TEST(Cp1_);
  TEST(Cp2_);
  TEST(Cint);
  TEST(C4__);
  TEST(C4_2);
  TEST(C__z);
  return 0;
}
