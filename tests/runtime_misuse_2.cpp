#include <stdio.h>
#include <emscripten.h>

extern "C" {

int noted = 0;

char* EMSCRIPTEN_KEEPALIVE note(int n) {
  EM_ASM_({ Module.noted = $0 }, (int)&noted);
  EM_ASM_({ Module.print([$0, $1]) }, n, noted);
  noted += n;
  EM_ASM_({ Module.print(['noted is now', $0]) }, noted);
  return (char*)"silly-string";
}

void free(void*) { // free is valid to call even after the runtime closes, so useful as a hack here for this test
  EM_ASM_({ Module.print(['reporting', $0]) }, noted);
}

}

