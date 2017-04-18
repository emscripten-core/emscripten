#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
class MyFoo {
public:
    MyFoo() {
        EM_ASM({print("constructing my foo");});
    }
    void doit() {
        EM_ASM({print("doing it");});
    }
};
EMSCRIPTEN_BINDINGS(my_module) {
    class_<MyFoo>("MyFoo")
      .constructor<>()
      .function("doit", &MyFoo::doit)
      ;
}
int main(int argc, char **argv) {
    EM_ASM(
      try {
        var foo = new Module.MyFoo();
        foo.doit();
      } catch(e) {
        Module.print(e);
      }
    );
    return 0;
}
