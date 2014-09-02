#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

// Toggle to use two different methods for updating shared data (C++03 volatile vs explicit atomic ops).
//#define USE_C_VOLATILE

volatile int sharedVar = 0;

static void *thread_start(void *arg) // thread: just flip the shared flag and quit.
{
#ifdef USE_C_VOLATILE
  sharedVar = 1;
#else
  emscripten_atomic_store_u32((void*)&sharedVar, 1);
#endif
  pthread_exit(0);
}

int main()
{
  pthread_t thr;
  pthread_create(&thr, NULL, thread_start, (void*)0);

#ifdef USE_C_VOLATILE
  while(sharedVar == 0) {
    EM_ASM(Module['print']('Main: HACKHACK. Without this print, the main thread will never observe sharedVar being set by the thread and this will loop indefinitely!'););
  }
#else
  while(emscripten_atomic_load_u32((void*)&sharedVar) == 0) {}
#endif

#ifdef REPORT_RESULT
  int result = sharedVar;
  REPORT_RESULT();
#endif
}
