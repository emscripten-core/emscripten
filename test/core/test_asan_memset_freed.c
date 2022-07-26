#include <stdlib.h>
#include <string.h>

int main(void) {
  void *ptr = malloc(64);
  free(ptr);
  memset(ptr, 0, 64);
}
