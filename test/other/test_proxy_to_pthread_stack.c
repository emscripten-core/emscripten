#define _GNU_SOURCE
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <emscripten.h>

int main(void) {
  pthread_attr_t attr;
  int rtn = pthread_getattr_np(pthread_self(), &attr);
  assert(rtn == 0);
  size_t stacksize = 0;
  rtn = pthread_attr_getstacksize(&attr, &stacksize);
  assert(rtn == 0);
  printf("stack size %zd\n", stacksize);

  // This test is run with STACK_SIZE=128k so we always expect that to be
  // the ammount of stack we have in main()
  assert(stacksize == 128*1024);

  // This test is run with DEFAULT_PTHREAD_STACK_SIZE=64k.  This would fail if
  // this thread were run with only 64k of stack.
  int8_t data[65*1024];
  memset(data, 0xa0, sizeof(data));
  EM_ASM(checkStackCookie());
  printf("data address %p\n", data);

  printf("success\n");
  return 0;
}
