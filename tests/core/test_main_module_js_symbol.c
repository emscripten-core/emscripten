#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <emscripten.h>

extern void jsPrintHello();

void (*fnPtr)() = &jsPrintHello;

int main(int argc, char* argv[]) {
  // Indirect call to jsPrintHello
  fnPtr();
  return 0;
}
