#include <emscripten/bind.h>
#include <iostream>
#include <memory>

class Foo {
  std::string mName;

public:
  Foo(std::string name) : mName(name) {}
  ~Foo() { std::cout << mName << " destructed" << std::endl; }
};

std::shared_ptr<Foo> foo() {
  return std::make_shared<Foo>("Constructed from C++");
}

Foo* pFoo() { return new Foo("Foo*"); }

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Marci) {
  class_<Foo>("Foo").smart_ptr_constructor<std::shared_ptr<Foo>>(
    "Foo", &std::make_shared<Foo, std::string>);

  function("foo", foo);
  function("pFoo", pFoo, allow_raw_pointers());
}
