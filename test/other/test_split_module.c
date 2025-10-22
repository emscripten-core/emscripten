#include <stdio.h>
#include <emscripten.h>
#include <stdlib.h> // For malloc and free
#include <stdint.h> // For int64_t

int foo() {
  return 42;
}

EMSCRIPTEN_KEEPALIVE void say_hello() {
  printf("Hello! answer: %d\n", foo());

  int64_t* ptr = (int64_t*)malloc(sizeof(int64_t) * 10);
  if (ptr == NULL) {
    return;
  }
  
  free(ptr);
  ptr = NULL;
}
