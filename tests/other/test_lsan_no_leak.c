#include <stdlib.h>

void *global_ptr;

int main(int argc, char **argv) {
  void *local_ptr = malloc(42);
  free(local_ptr);
  global_ptr = malloc(1337);
  free(malloc(2048));
}
