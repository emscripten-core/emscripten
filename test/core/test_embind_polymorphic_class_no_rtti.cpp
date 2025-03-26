// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <stdio.h>

EM_JS(void, calltest, (), {
  var foo = new Module.Foo();
  var fooRaw = Module.FooRaw();
  console.log("foo.test() returned: " + foo.test());
  console.log("foo.getClassName() -> " + foo.getClassName());
  console.log("fooRaw.getClassName() -> " + fooRaw.getClassName());
#if __has_feature(cxx_rtti)
  // When RTTI is available we do automatic downcasting
  assert(foo.getClassName() == "Bar");
  assert(fooRaw.getClassName() == "Bar");
#else
  // When RTTI is not available we cannot do automatic downcasting
  assert(foo.getClassName() == "Foo");
  assert(fooRaw.getClassName() == "Foo");
#endif
  foo.delete();
  fooRaw.delete();
});

class Foo {
  public:
  virtual ~Foo() = default;
  virtual int test() = 0;
  std::string getClassName() {
    return "Foo";
  }
};

class Bar : public Foo {
  public:
  int test() override { return 42; }
  std::string getClassName() {
    return "Bar";
  }
};

int main(int argc, char** argv){
  printf("in main\n");
  calltest();
  printf("calltest done\n");
  return 0;
}

Foo* makeFooRaw() {
  return new Bar();
}

std::shared_ptr<Foo> makeFoo() {
  return std::make_shared<Bar>();
}

using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_module) {
  function("FooRaw", &makeFooRaw, allow_raw_pointers());
  emscripten::class_<Foo>("Foo")
    .smart_ptr_constructor("Foo", &makeFoo)
    .function("test", &Foo::test)
    .function("getClassName", &Foo::getClassName)
    ;
  class_<Bar, base<Foo>>("Bar")
    .function("getClassName", &Bar::getClassName);
};
