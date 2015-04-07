#include <stdlib.h>
int main() {
  void* p = malloc(42);
  void* q = malloc(43);
  free(p);
}

