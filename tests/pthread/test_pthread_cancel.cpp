#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <emscripten.h>
#include <emscripten/threading.h>

volatile int res = 43;
static void cleanup_handler(void *arg)
{
  EM_ASM_INT( { Module['print']('Called clean-up handler with arg ' + $0); }, arg);
  int a = (int)arg;
  res -= a;
}

static void *thread_start(void *arg)
{
  pthread_cleanup_push(cleanup_handler, (void*)42);
  EM_ASM(Module['print']('Thread started!'););
  for(;;)
  {
    pthread_testcancel();
  }
  res = 1000; // Shouldn't ever reach here.
  pthread_cleanup_pop(0);
}

pthread_t thr;

int main()
{
  int result;
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    result = 1;
    REPORT_RESULT();
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  int s = pthread_create(&thr, NULL, thread_start, (void*)0);
  assert(s == 0);
  EM_ASM(Module['print']('Canceling thread..'););
  s = pthread_cancel(thr);
  assert(s == 0);

  for(;;)
  {
    result = emscripten_atomic_load_u32((const void*)&res);
    if (result == 1)
    {
      EM_ASM_INT( { Module['print']('After canceling, shared variable = ' + $0 + '.'); }, result);
#ifdef REPORT_RESULT
      REPORT_RESULT();
#endif
      return 0;
    }
  }
}
