#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int f(char *x) {
  int z = *x;
  free(x);
  return z;
}

int main() {
  char *x = malloc(10);
  static char y[10];
  x[0] = 0;
  strcpy(x, "hello");
  memchr("hello", 0, 6);
  strchr("hello", 'z');
  strlen("hello");

  // setlocale is needed here otherwise mbsrtowcs takes the fast path
  // via strcpy.
  setlocale(LC_ALL, "en_US.UTF-8");
  const char* s = "abcd";
  mbsrtowcs(NULL, &s, 0, NULL);

  printf("%d %d\n", f(x), y[9]);
  printf("done\n");
  return 0;
}
