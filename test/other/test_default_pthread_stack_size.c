#include <assert.h>
#include <stdio.h>
#include <pthread.h>

#ifndef EXPECTED_STACK_SIZE
// Should match DEFAULT_PTHREAD_STACK_SIZE = 64*1024;
#define EXPECTED_STACK_SIZE (64*1024)
#endif

int main() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  size_t stacksize;
  pthread_attr_getstacksize(&attr, &stacksize);
  printf("expected: %d, actual: %zu\n", EXPECTED_STACK_SIZE, stacksize);
  assert(stacksize == EXPECTED_STACK_SIZE);
  printf("done\n");
  return 0;
}
