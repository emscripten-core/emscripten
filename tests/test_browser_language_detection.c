#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main(int argc, char** argv) {
  printf("%s\n", getenv("LANG"));

#ifndef __EMSCRIPTEN__
  // emscripten has no locale support in *scanf as of 2019-06
  char* cur_locale = setlocale(LC_ALL, "");
  printf("locale: %s\n", cur_locale);
  float ret = 0;
  sscanf("1,2", "%f", &ret);
  printf("%f\n", ret);
  sscanf("3.4", "%f", &ret);
  printf("%f\n", ret);
  
  // Expected results:
  // LANG=C: 1 / 3.4
  // LANG=fr_FR.UTF-8: 1,2 / 3
  // Note: locale needs to be installed (cf. `locale -a`)
#endif
}
