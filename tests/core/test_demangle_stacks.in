#include <stdio.h>
#include <stdlib.h>

namespace NameSpace {
class Class {
 public:
  void Aborter(double x, char y, int *z) {
    int addr = x + y + (int)z;
    void *p = (void *)addr;
    for (int i = 0; i < 100; i++)
      free(p);  // will abort, should show proper stack trace
  }
};
}

int main(int argc, char **argv) {
  NameSpace::Class c;
  c.Aborter(1.234, 'a', NULL);
  return 0;
}
