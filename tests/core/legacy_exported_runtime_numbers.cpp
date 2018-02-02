#include<emscripten.h>

int main() {
#ifdef DIRECT
  EM_ASM({
    Module['print']('|' + ALLOC_DYNAMIC + '|');
  });
#else
  EM_ASM({
    Module['print']('|' + Module['ALLOC_DYNAMIC'] + '|');
  });
#endif
}

