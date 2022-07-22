#include <string.h>
#include <stdlib.h>

int main(void) {
  char *hello = malloc(5);
  strcpy(hello, "hello");
  return hello[4];
}
