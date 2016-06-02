#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <vector>

#define NUM_BLOCKS_TO_ALLOC 50000
#define NUM_THREADS 8

pthread_mutex_t vectorMutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<void*> allocatedMemory;

static void *thread_start(void *arg)
{
  for(int i = 0; i < NUM_BLOCKS_TO_ALLOC; ++i)
  {
    void *mem = malloc(4);
    pthread_mutex_lock(&vectorMutex);
    allocatedMemory.push_back(mem);
    pthread_mutex_unlock(&vectorMutex);
  }
  pthread_exit(0);
}

int main()
{
  int result = 0;
  if (!emscripten_has_threading_support()) {
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    printf("Skipped: threading support is not available!\n");
    return 0;
  }
  pthread_t thr[NUM_THREADS];
  for(int i = 0; i < NUM_THREADS; ++i)
  {
    int rc = pthread_create(&thr[i], NULL, thread_start, 0);
    if (rc != 0)
    {
#ifdef REPORT_RESULT
      result = (rc != EAGAIN);
      REPORT_RESULT();
      return 0;
#endif
    }
  }
  unsigned long numBlocksToFree = NUM_BLOCKS_TO_ALLOC * NUM_THREADS;
  while(numBlocksToFree > 0)
  {
    pthread_mutex_lock(&vectorMutex);
    for(size_t i = 0; i < allocatedMemory.size(); ++i)
      free(allocatedMemory[i]);
    numBlocksToFree -= allocatedMemory.size();
    allocatedMemory.clear();
    pthread_mutex_unlock(&vectorMutex);
  }
  for(int i = 0; i < NUM_THREADS; ++i)
  {
    int res = 0;
    int rc = pthread_join(thr[i], (void**)&res);
    assert(rc == 0);
    assert(res == 0);
  }
  printf("Test finished successfully!\n");
#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}
