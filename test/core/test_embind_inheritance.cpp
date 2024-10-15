// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/em_asm.h>
#include <emscripten/console.h>
#include <emscripten/bind.h>

using namespace emscripten;


class MyFoo {
public:
  MyFoo() {
    emscripten_out("constructing my foo");
  }
  virtual void doit() {
    emscripten_out("doing it");
  }
  virtual ~MyFoo() {
    emscripten_out("destructing my foo");
  }
};

class MyBar : public MyFoo {
public:
  MyBar() {
    emscripten_out("constructing my bar");
  }
  void doit() override {
    emscripten_out("doing something else");
  }
  virtual ~MyBar() override {
    emscripten_out("destructing my bar");
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
      var foo = new Module['MyFoo']();
      foo['doit']();
      var bar = new Module['MyBar']();
      bar['doit']();
    } catch(e) {
      out(e);
    } finally {
      foo.delete();
      bar.delete();
    }
  );
  return 0;
}
