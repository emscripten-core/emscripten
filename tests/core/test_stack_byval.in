// We should also not blow up the stack with byval arguments
#include <stdio.h>
struct vec {
  int x, y, z;
  vec(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
  static vec add(vec a, vec b) { return vec(a.x + b.x, a.y + b.y, a.z + b.z); }
};
int main() {
  int total = 0;
  for (int i = 0; i < 1000; i++) {
    for (int j = 0; j < 1000; j++) {
      vec c(i + i % 10, j * 2, i % 255);
      vec d(j * 2, j % 255, i % 120);
      vec f = vec::add(c, d);
      total += (f.x + f.y + f.z) % 100;
      total %= 10240;
    }
  }
  printf("sum:%d*\n", total);
  return 0;
}
