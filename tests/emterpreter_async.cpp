#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

int main() {
  // infinite main loop, turned async using emterpreter
  int counter = 0;
  while (1) {
    printf("frame: %d\n", ++counter);
    emscripten_sleep(100);
    if (counter == 10) {
      int result = 1;
      REPORT_RESULT();
      break;
    }
  }
}

