#include <stdio.h>
#include <locale.h>

int main(int argc, char* argv[]) {
  char* rtn;
  rtn = setlocale(LC_ALL, "C");
  printf("done setlocale 'C': '%s'\n", rtn);
  rtn = setlocale(LC_ALL, "waka");
  printf("done setlocale 'waka': '%s'\n", rtn);
  return 0;
}
