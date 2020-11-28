#include <stdio.h>
#include <emscripten.h>

void later_print(int param) {
  printf("later print: %d\n", param);
}

EMSCRIPTEN_KEEPALIVE
extern "C"
void print(int param) {
  // The third call and onwards will call extra code.
  if (param < 3) {
    // Use puts early, and printf later
    puts("early print");
  } else {
    emscripten_lazy_load_code();
    later_print(param);
  }
  EM_ASM({
    if (Module.callback) Module.callback();
  });
}

int main() {
  EM_ASM({
    var num = 0;
    // Use a callback, so that we can call the export at the proper times - the
    // lazy load call will be asynchronous in when it completes.
    Module.callback = function() {
      // Do the call via an async callback, to avoid recursion. Each call to
      // the export will be effectively from the top level, which is more
      // realistic.
      setTimeout(function() {
        if (num === 6) {
          Module.callback = null;
          out("all done");
          return;
        }
        console.log("print", num);
        _print(num);
        num++;
      }, 0);
    };
    Module.callback();
  });
}
