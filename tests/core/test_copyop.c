#include <stdio.h>
#include <math.h>
#include <string.h>

struct vec {
  double x, y, z;
  vec() : x(0), y(0), z(0) {};
  vec(const double a, const double b, const double c) : x(a), y(b), z(c) {};
};

struct basis {
  vec a, b, c;
  basis(const vec& v) {
    a = v;  // should not touch b!
    printf("*%.2f,%.2f,%.2f*\n", b.x, b.y, b.z);
  }
};

int main() {
  basis B(vec(1, 0, 0));

  // Part 2: similar problem with memset and memmove
  int x = 1, y = 77, z = 2;
  memset((void*)&x, 0, sizeof(int));
  memset((void*)&z, 0, sizeof(int));
  printf("*%d,%d,%d*\n", x, y, z);
  memcpy((void*)&x, (void*)&z, sizeof(int));
  memcpy((void*)&z, (void*)&x, sizeof(int));
  printf("*%d,%d,%d*\n", x, y, z);
  memmove((void*)&x, (void*)&z, sizeof(int));
  memmove((void*)&z, (void*)&x, sizeof(int));
  printf("*%d,%d,%d*\n", x, y, z);
  return 0;
}
