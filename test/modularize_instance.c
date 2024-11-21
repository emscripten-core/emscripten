#include <stdio.h>
#include <emscripten.h>
#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#include <string.h>
#endif

EMSCRIPTEN_KEEPALIVE void foo() {
  printf("foo\n");
}

void bar() {
  printf("bar\n");
}

void *thread_function(void *arg) {
  printf("main2\n");
  return NULL;
}

int main() {
  printf("main1\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t thread_id;
  int result = pthread_create(&thread_id, NULL, thread_function, NULL);
  if (result != 0) {
    fprintf(stderr, "Error creating thread: %s\n", strerror(result));
    return 1;
  }
  pthread_join(thread_id, NULL); 
#else
  printf("main2\n");
#endif
  return 0;
}
