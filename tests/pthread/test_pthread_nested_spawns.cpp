#include <stdio.h>
#include <pthread.h>
#include <emscripten/threading.h>

int result = 0;

static void *thread2_func(void *vptr_args) {
  puts("c");
  result = 1;
  return NULL;
}

static void *thread_func(void *vptr_args) {
  pthread_t thread;
  puts("b");
  pthread_create(&thread, NULL, thread2_func, NULL);
  pthread_join(thread, NULL);
  return NULL;
}

int main(void) {
  pthread_t thread;
  puts("a");
  pthread_create(&thread, NULL, thread_func, NULL);
  if (emscripten_has_threading_support()) {
    pthread_join(thread, NULL);
  } else {
    result = 1;
  }

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
  return 0;
}
