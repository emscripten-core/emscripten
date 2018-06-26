#include <emscripten.h>

int main() {
  volatile int i = 1;
  volatile int j = 0;
  EM_ASM({
    out('|' + $0 + '|')
  }, i / j);
}

