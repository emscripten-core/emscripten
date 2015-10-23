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

}

int main() {
  EM_ASM( myJSCallback() ); // calls a global JS func
  return 0;
}

