#include <stdio.h>
struct Pure {
  virtual int implme() = 0;
};
struct Parent : Pure {
  virtual int getit() {
    return 11;
  };
  int implme() { return 32; }
};
struct Child : Parent {
  int getit() { return 74; }
  int implme() { return 1012; }
};

struct Other {
  int one() { return 11; }
  int two() { return 22; }
  virtual int three() { return 33; }
  virtual int four() { return 44; }
};

int main() {
  Parent *x = new Parent();
  Parent *y = new Child();
  printf("*%d,%d,%d,%d*\n", x->getit(), y->getit(), x->implme(), y->implme());

  Other *o = new Other;

  int (Other::*Ls)() = &Other::one;
  printf("*%d*\n", (o->*(Ls))());
  Ls = &Other::two;
  printf("*%d*\n", (o->*(Ls))());

  Ls = &Other::three;
  printf("*%d*\n", (o->*(Ls))());
  Ls = &Other::four;
  printf("*%d*\n", (o->*(Ls))());

  return 0;
}
