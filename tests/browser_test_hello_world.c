#include<stdio.h>

#include<emscripten.h>

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
    assert(Module.prints.length === 1, 'bad length');
    assert(Module.prints[0] == 'hello, world!', 'bad contents: ' + Module.prints[0]);
  });
  int result = 0;
  REPORT_RESULT();
  return 0;
}

