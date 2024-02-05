#include <string.h>
#include <stdlib.h>

int main(void) {
  char *hello = malloc(4);
  memcpy(hello, "hello", 5);
  return hello[3];
}
