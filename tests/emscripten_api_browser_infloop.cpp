#include <stdio.h>
#include <string.h>
#include <emscripten.h>

struct Class {
  static Class *instance;

  int x;

  Class() : x(0) {}

  void print() {
    char buf[18];
    memset(buf, 0, 18); // clear stack. if we did not simulate infinite loop, this clears x and is a bug!
    x += buf[7];

    printf("waka %d\n", x++);

    if (x == 7) {
      int result = x;
      REPORT_RESULT();
    }
  }

  static void callback() {
    instance->print();
  }

  void start() {
    instance = this;
    emscripten_set_main_loop(Class::callback, 3, 1); // important if we simulate an infinite loop here or not
  }
};

Class *Class::instance = NULL;

int main() {
  Class().start();
  return 1;
}

