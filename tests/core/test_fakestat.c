#include <stdio.h>
struct stat {
  int x, y;
};
int main() {
  stat s;
  s.x = 10;
  s.y = 22;
  printf("*%d,%d*\n", s.x, s.y);
}
