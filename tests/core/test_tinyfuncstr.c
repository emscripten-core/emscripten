#include <stdio.h>

struct Class {
  static char *name1() { return "nameA"; }
  char *name2() { return "nameB"; }
};

int main() {
  printf("*%s,%s*\n", Class::name1(), (new Class())->name2());
  return 0;
}
