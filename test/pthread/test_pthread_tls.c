#include <pthread.h>
#include <stdio.h>
#include <assert.h>

_Thread_local int tls;
_Thread_local struct {
  int a;
  double b;
} data = {1, 2};
_Thread_local int array[10];

void* thread_main(void* arg) {
  ++tls;
  data.a = 3;
  data.b = 4;
  assert(tls == 1);
  assert(data.a == 3);
  assert(data.b == 4);
  assert(array[9] == 0);
  return NULL;
}

int main(void) {
  array[9] = 1337;
  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);
  assert(tls == 0);
  assert(data.a == 1);
  assert(data.b == 2);
  assert(array[9] == 1337);
  return 0;
}
