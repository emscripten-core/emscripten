#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 8
#define N 6

static void *thread_start(void *arg)
{
  int n = (int)arg;
  int *mem[N] = {};
  for(int i = 0; i < N; ++i)
  {
    mem[i] = (int*)malloc(4);
    *mem[i] = n+i;
  }
  for(int i = 0; i < N; ++i)
  {
    int k = *mem[i];
    if (k != n+i)
    {
      EM_ASM_INT( { console.error('Memory corrupted! mem[i]: ' + $0 + ', i: ' + $1 + ', n: ' + $2); }, k, i, n);
      pthread_exit((void*)1);
    }

    assert(*mem[i] == n+i);
    free(mem[i]);
  }
  EM_ASM_INT( { console.log('Worker with task number ' + $0 + ' finished.'); }, n);
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
    pthread_create(&thr[i], NULL, thread_start, (void*)(i*N));
  for(int i = 0; i < NUM_THREADS; ++i) {
    int res = 0;
    pthread_join(thr[i], (void**)&res);
    result += res;
  }
  printf("Test finished with result %d\n", result);

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}
