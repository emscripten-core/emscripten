#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <stdlib.h>

volatile int result = 0;

static void *thread2_start(void *arg)
{
  result = 1;
  pthread_exit(0);
}

static void *thread1_start(void *arg)
{
  pthread_t thr;
  pthread_create(&thr, NULL, thread2_start, 0);
  pthread_join(thr, 0);
  pthread_exit(0);
}

int main()
{
  pthread_t thr;
  pthread_create(&thr, NULL, thread1_start, 0);
  pthread_join(thr, 0);

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}
