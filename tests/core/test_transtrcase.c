#include <stdio.h>
#include <string.h>
int main() {
  char szToupr[] = "hello, ";
  char szTolwr[] = "EMSCRIPTEN";
  strupr(szToupr);
  strlwr(szTolwr);
  puts(szToupr);
  puts(szTolwr);
  return 0;
}
