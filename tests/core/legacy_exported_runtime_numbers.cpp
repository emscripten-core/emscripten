#include<emscripten.h>

int main() {
#ifdef DIRECT
  EM_ASM({
    out('|' + ALLOC_DYNAMIC + '|');
  });
#else
  EM_ASM({
    out('|' + Module['ALLOC_DYNAMIC'] + '|');
  });
#endif
}

