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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#include <emscripten/eventloop.h>
#include <emscripten/wasm_worker.h>
#endif

char *char_repeat(int n, char c) {
  char *dest = malloc(n + 1);
  memset(dest, c, n);
  dest[n] = '\0';
  return dest;
}

void thread_func() {
  char *msg = char_repeat(100, 'a');
  for (int i = 0; i < 10; ++i) {
    printf("%s\n", msg);
  }
  free(msg);
}

#if defined(__EMSCRIPTEN_PTHREADS__)
void *thread_main(void *arg) {
  thread_func();
  return 0;
}
#endif

#if defined(__EMSCRIPTEN_WASM_WORKERS__)
void terminate_worker(void* userData) {
  emscripten_terminate_all_wasm_workers();
  printf("main done\n");
}
#endif

int main() {
  printf("in main\n");
#ifdef __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_t worker[2];
  worker[0] = emscripten_malloc_wasm_worker(/*stack size: */ 4096);
  worker[1] = emscripten_malloc_wasm_worker(/*stack size: */ 4096);
  emscripten_wasm_worker_post_function_v(worker[0], thread_func);
  emscripten_wasm_worker_post_function_v(worker[1], thread_func);

  // Terminate both workers after a small delay
  emscripten_set_timeout(terminate_worker, 1000, 0);
#else
  pthread_t thread[2];
  void *thread_rtn;
  int rc;

  rc = pthread_create(&thread[0], NULL, thread_main, NULL);
  assert(rc == 0);

  rc = pthread_create(&thread[1], NULL, thread_main, NULL);
  assert(rc == 0);

  rc = pthread_join(thread[0], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);

  rc = pthread_join(thread[1], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);
  printf("main done\n");
#endif
  return 0;
}
