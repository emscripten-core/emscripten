#include <stdio.h>
#include <emscripten.h>

extern "C" {

int noted = 1;

char* EMSCRIPTEN_KEEPALIVE note(int n) {
  EM_ASM({ Module.print([$0, $1]) }, n, noted);
  noted = noted | n;
  EM_ASM({ Module.print(['noted is now', $0]) }, noted);
  if (noted == 3) {
    REPORT_RESULT(noted);
  }
  return "silly-string";
}

}

int main() {
  EM_ASM( myJSCallback() ); // calls a global JS func
  return 0;
}

