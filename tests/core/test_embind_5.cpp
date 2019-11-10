// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;


class MyFoo {
public:
  MyFoo() {
    EM_ASM({out("constructing my foo");});
  }
  virtual void doit() {
    EM_ASM({out("doing it");});
  }
};

class MyBar : public MyFoo {
public:
  MyBar() {
    EM_ASM({out("constructing my bar");});
  }
  void doit() override {
    EM_ASM({out("doing something else");});
  }
};


EMSCRIPTEN_BINDINGS(my_module) {
  class_<MyFoo>("MyFoo")
    .constructor<>()
    .function("doit", &MyFoo::doit)
    ;
  class_<MyBar, base<MyFoo>>("MyBar")
    .constructor<>()
    ;
}


int main(int argc, char **argv) {
  EM_ASM(
    try {
      var foo = new Module.MyFoo();
      foo.doit();
      var bar = new Module.MyBar();
      bar.doit();
    } catch(e) {
      out(e);
    }
  );
  return 0;
}
