// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
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
      emscripten_cancel_main_loop();
      exit(x);
    }
  }

  static void callback() {
    instance->print();
  }

  void start() {
    instance = this;

    EM_ASM({
      var initial = stackSave();
      out('seeing initial stack of ' + initial);
      setTimeout(function() {
        var current = stackSave();
        out('seeing later stack of   ' + current);
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

