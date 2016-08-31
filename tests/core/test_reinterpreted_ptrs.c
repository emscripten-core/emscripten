#include <stdio.h>

class Foo {
 private:
  float bar;

 public:
  int baz;

  Foo() : bar(0), baz(4711) {};

  int getBar() const;
};

int Foo::getBar() const {
  return this->bar;
};

const Foo *magic1 = reinterpret_cast<Foo *>(0xDEAD111F);
const Foo *magic2 = reinterpret_cast<Foo *>(0xDEAD888F);

static void runTest() {

  const Foo *a = new Foo();
  const Foo *b = a;

  if (a->getBar() == 0) {
    if (a->baz == 4712)
      b = magic1;
    else
      b = magic2;
  }

  printf("%s\n",
         (b == magic1 ? "magic1" : (b == magic2 ? "magic2" : "neither")));
};

extern "C" {
int main(int argc, char **argv) { runTest(); }
}
