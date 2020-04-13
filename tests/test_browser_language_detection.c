#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main(int argc, char** argv) {
  if (getenv("LANG")) {
    printf("%s\n", getenv("LANG"));
  }

#ifndef __EMSCRIPTEN__
  // Emscripten has no locale support in *scanf as of 2019-06.
  // These tests for future use - or native/desktop testing:
  char* cur_locale = setlocale(LC_ALL, "");
  printf("locale: %s\n", cur_locale);
  float ret = 0;
  sscanf("1,2", "%f", &ret);
  printf("%f\n", ret);
  sscanf("3.4", "%f", &ret);
  printf("%f\n", ret);
  
  // Expected results:
  // LANG=C ./a.out: 1 / 3.4
  // LANG=fr_FR.UTF-8 ./a.out: 1,2 / 3
  // Note: requested locale needs to be installed locally (cf. `locale -a`)
  //       otherwise setlocale(3) is no-op
#endif
}
