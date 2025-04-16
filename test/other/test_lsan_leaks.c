#include <stdlib.h>

void *global_ptr;

void f(void) {
  void *local_ptr = malloc(42);
}

int main(int argc, char **argv) {
  global_ptr = malloc(1337);
  global_ptr = 0;
  f();
  malloc(2048);
}

#ifdef DISABLE_CONTEXT
const char *__lsan_default_options(void) {
  return "malloc_context_size=0";
}
#endif
