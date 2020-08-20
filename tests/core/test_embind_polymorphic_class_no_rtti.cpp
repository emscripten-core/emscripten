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
  console.log("foo.test() returned: " + foo.test());
});

class Foo {
  public:
  virtual ~Foo() = default;
  virtual int test() = 0;
};

class Bar : public Foo {
  public:
  int test() override { return 42; }
};

int main(int argc, char** argv){
  printf("Hello, world.\n");
  calltest();
  return 0;
}

std::shared_ptr<Foo> MakeFoo() {
  return std::make_shared<Bar>();
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::class_<Foo>("Foo")
    .smart_ptr_constructor("Foo", &MakeFoo)
    .function("test", &Foo::test)
    ;
};
