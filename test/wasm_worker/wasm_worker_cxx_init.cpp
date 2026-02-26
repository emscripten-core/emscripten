#include <stdio.h>

struct Foo {
  Foo() {
    printf("create Foo\n");
  }

  ~Foo() {
    printf("destroy Foo\n");
  }

  void baz() {
    printf("method called\n");
  }
};


void bar() {
  static Foo foo;
  foo.baz();
}

int main() {
  printf("main\n");
  bar();
  printf("done\n");
  return 0;
}
