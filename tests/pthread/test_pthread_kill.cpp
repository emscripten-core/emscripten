#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <emscripten.h>
#include <emscripten/threading.h>

volatile int sharedVar = 0;

static void *thread_start(void *arg)
{
  // As long as this thread is running, keep the shared variable latched to nonzero value.
  for(;;)
  {
    ++sharedVar;
    emscripten_atomic_store_u32((void*)&sharedVar, sharedVar+1);
  }

  pthread_exit(0);
}

pthread_t thr;

void BusySleep(double msecs)
{
  double t0 = emscripten_get_now();
  while(emscripten_get_now() < t0 + msecs);
}

int main()
{
  int result;
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    result = 0;
    REPORT_RESULT();
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  sharedVar = 0;
  int s = pthread_create(&thr, NULL, thread_start, 0);
  assert(s == 0);

  // Wait until thread kicks in and sets the shared variable.
  while(sharedVar == 0)
    BusySleep(10);

  s = pthread_kill(thr, SIGKILL);
  assert(s == 0);

  // Wait until we see the shared variable stop incrementing. (This is a bit heuristic and hacky)
  for(;;)
  {
    int val = emscripten_atomic_load_u32((void*)&sharedVar);
    BusySleep(100);
    int val2 = emscripten_atomic_load_u32((void*)&sharedVar);
    if (val == val2) break;
  }

  // Reset to 0.
  sharedVar = 0;
  emscripten_atomic_store_u32((void*)&sharedVar, 0);

  // Wait for a long time, if the thread is still running, it should progress and set sharedVar by this time.
  BusySleep(3000);

  // Finally test that the thread is not doing any work and it is dead.
  assert(sharedVar == 0);
  assert(emscripten_atomic_load_u32((void*)&sharedVar) == 0);
  EM_ASM_INT( { Module['print']('Main: Done. Successfully killed thread. sharedVar: '+$0+'.'); }, sharedVar);
#ifdef REPORT_RESULT
  result = sharedVar;
  REPORT_RESULT();
#endif
}
