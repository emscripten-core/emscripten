#include <emscripten.h>

int main() {
  {
    volatile double d = 17179870521;
    EM_ASM({
      out('|' + $0 + '|')
    }, int(d));
  }
  {
    // unsigned
    volatile double d = 4294967295.0;
    EM_ASM({
      out('|' + ($0 >>> 0) + '|')
    }, unsigned(d));
  }
}

