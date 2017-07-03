#include <stdio.h>
#include <stdlib.h>

namespace NameSpace {
class Class {
 public:
  __attribute__((noinline))
  void Aborter(double x, char y, int *z) {
    volatile int w = 1;
    if (w) {
      abort();
    }
  }
};
}

int main(int argc, char **argv) {
  NameSpace::Class c;
  c.Aborter(1.234, 'a', NULL);
  return 0;
}
