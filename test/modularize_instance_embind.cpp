#include <stdio.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

void foo() {
  printf("foo\n");
}

struct Bar {
  void print() {
    printf("bar\n");
  }
};

int main() {
  printf("main\n");
}

EMSCRIPTEN_BINDINGS(xxx) {
  function("foo", &foo);
  class_<Bar>("Bar")
    .constructor<>()
    .function("print", &Bar::print);
}
