/*
 * Regression test for stdio locking.  If file locking is not enabled the
 * threads will race to write the file output buffer and we will see lines
 * that are longer or shorter then 100 characters.  When locking is
 * working/enabled each 100 character line will be printed separately.
 *
 * See:
 *   musl/src/stdio/__lockfile.c
 *   musl/src/stdio/fwrite.c
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>

pthread_t thread[2];
#elif defined(__EMSCRIPTEN_WASM_WORKERS__)
#include <emscripten/wasm_worker.h>
#include <emscripten/eventloop.h>

emscripten_wasm_worker_t worker[2];
#else
#error Expected to be compiled with either -sWASM_WORKERS or -pthread.
#endif

#ifndef __wasm_atomics__
#error Expected to be compiled with -matomics.
#endif

#ifndef __wasm_bulk_memory__
#error Expected to be compiled with -mbulk-memory.
#endif

char *char_repeat(int n, char c) {
  char *dest = malloc(n + 1);
  memset(dest, c, n);
  dest[n] = '\0';
  return dest;
}

void thread_main() {
  char *msg = char_repeat(100, 'a');
  for (int i = 0; i < 10; ++i)
    printf("%s\n", msg);
  free(msg);
}

#ifdef __EMSCRIPTEN_WASM_WORKERS__
void terminate_worker(void *userData)
{
  emscripten_terminate_all_wasm_workers();
  printf("main done\n");
}
#endif

int main() {
  printf("in main\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  void *thread_rtn;
  int rc;

  rc = pthread_create(&thread[0], NULL, (void* (*)(void*))thread_main, NULL);
  assert(rc == 0);

  rc = pthread_create(&thread[1], NULL, (void* (*)(void*))thread_main, NULL);
  assert(rc == 0);

  rc = pthread_join(thread[0], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);

  rc = pthread_join(thread[1], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);

  printf("main done\n");
#else
  worker[0] = emscripten_malloc_wasm_worker(/*stack size: */1024);
  worker[1] = emscripten_malloc_wasm_worker(/*stack size: */1024);
  emscripten_wasm_worker_post_function_v(worker[0], (void (*))thread_main);
  emscripten_wasm_worker_post_function_v(worker[1], (void (*))thread_main);

  // Terminate both workers after a small delay
  emscripten_set_timeout(terminate_worker, 1000, 0);
#endif
  return 0;
}
