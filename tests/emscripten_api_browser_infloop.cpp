#include <stdio.h>
#include <string.h>
#include <emscripten.h>

struct Class {
  static Class *instance;

  int x;

  Class() : x(0) {}
  ~Class() { x = -9999; }

  void print() {
    printf("waka %d\n", x++);

    if (x == 7 || x < 0) {
      int result = x;
      REPORT_RESULT();
      emscripten_cancel_main_loop();
    }
  }

  static void callback() {
    instance->print();
  }

  void start() {
    instance = this;

    EM_ASM({
      var initial = Runtime.stackSave();
      Module.print('seeing initial stack of ' + initial);
      setTimeout(function() {
        var current = Runtime.stackSave();
        Module.print('seeing later stack of   ' + current);
        assert(current === initial);
      }, 0);
    });

    // important if we simulate an infinite loop here or not. With an infinite loop, the
    // destructor should *NOT* have been called
    emscripten_set_main_loop(Class::callback, 3, 1);
  }
};

Class *Class::instance = NULL;

int main() {
  Class().start();
  return 1;
}

