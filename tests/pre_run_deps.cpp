#include <stdio.h>
#include <emscripten.h>

int main() {
  printf("main() called.\n");
  int result = emscripten_run_script_int("Module.okk");
  REPORT_RESULT();
  return 1;
}

