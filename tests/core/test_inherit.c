#include <stdio.h>
struct Parent {
  int x1, x2;
};
struct Child : Parent {
  int y;
};
int main() {
  Parent a;
  a.x1 = 50;
  a.x2 = 87;
  Child b;
  b.x1 = 78;
  b.x2 = 550;
  b.y = 101;
  Child* c = (Child*)&a;
  c->x1++;
  c = &b;
  c->y--;
  printf("*%d,%d,%d,%d,%d,%d,%d*\n", a.x1, a.x2, b.x1, b.x2, b.y, c->x1, c->x2);
  return 0;
}
