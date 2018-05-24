#include <stdio.h>

struct Support {
  virtual void f() { printf("f()\n"); }
};

struct Derived : Support {};

int main() {
  Support* p = new Derived;
  dynamic_cast<Derived*>(p)->f();
}
