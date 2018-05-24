#include <stdio.h>

#include <emscripten.h>

int main() {
  EM_ASM({
    Module.prints = [];

    var real = Module['print'];

    Module['print'] = function(x) {
      real(x);
      Module.prints.push(x);
    }
  });
  printf("hello, world!\n");
  EM_ASM({
    if (Module.prints.length !== 1) throw 'bad length ' + Module.prints.length;
    if (Module.prints[0] !== 'hello, world!') throw 'bad contents: ' + Module.prints[0];
  });
  REPORT_RESULT(0);
  return 0;
}

