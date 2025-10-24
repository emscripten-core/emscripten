#include <stdio.h>
#include <emscripten.h>
#include <stdlib.h> // For malloc and free
#include <stdint.h> // For int64_t

int foo() {
  return 42;
}

EMSCRIPTEN_KEEPALIVE void say_hello() {
  printf("Hello! answer: %d\n", foo());

  void* ptr = malloc(10);
  if (ptr == NULL) {
    return;
  }
  
  free(ptr);
  ptr = NULL;
}
