#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>

int foo() {
  return 42;
}

void* thread(void* ptr) {
  printf("Hello! answer: %d from %d\n", foo(), (int)pthread_self());
  EM_ASM(makePrintProfile('in-thread.profile')());
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, thread, NULL);
  pthread_join(t, NULL);
  EM_ASM(makePrintProfile('main.profile')());
}
