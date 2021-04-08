#define _GNU_SOURCE
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int main(void) {
  pthread_attr_t attr;
  pthread_getattr_np(pthread_self(), &attr);
  size_t stacksize = 0;
  pthread_attr_getstacksize(&attr, &stacksize);
  printf("stack size %zd\n", stacksize);

  // Run with TOTAL_STACK=128k.
  assert(stacksize == 128*1024);

  // Run with DEFAULT_PTHREAD_STACK_SIZE=64k.
  // This would fail if we were actually running with only the default pthread stack size.
  int32_t data[64*1024];
  printf("data address %p\n", data);
  printf("success\n");
  exit(0);
}
