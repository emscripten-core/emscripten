#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;


class MyFoo {
public:
  MyFoo() {
    EM_ASM({Module.print("constructing my foo");});
  }
  virtual void doit() {
    EM_ASM({Module.print("doing it");});
  }
};

class MyBar : public MyFoo {
public:
  MyBar() {
    EM_ASM({Module.print("constructing my bar");});
  }
  void doit() override {
    EM_ASM({Module.print("doing something else");});
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
      Module.print(e);
    }
  );
  return 0;
}
