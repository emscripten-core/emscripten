#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/emmalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Tests that operating malloc from Wasm Workers is thread-safe.

emscripten_semaphore_t workDone = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(0);

// Make sure we have tiny byte of TLS data that causes LLVM TLS size to be not a multiple of four from the get-go.
__thread uint8_t dummyTls;

void work()
{
  assert(dummyTls == 0);
  for(int i = 0; i < 100000; ++i)
  {
    dummyTls += 1;
    void *ptr = malloc(emscripten_random() * 10000);
    assert(ptr);
    free(ptr);
  }
  printf("dummyTls: %d\n", (int)dummyTls);
  emscripten_semaphore_release(&workDone, 1);
}

void allThreadsDone(volatile void *address, uint32_t idx, ATOMICS_WAIT_RESULT_T result, void *userData)
{
#ifdef EMMALLOC
  assert(emmalloc_validate_memory_regions() == 0);
#endif
  printf("Test passed!\n");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main()
{
#define NUM_THREADS 10
  for(int i = 0; i < NUM_THREADS; ++i)
  {
    emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024);
    emscripten_wasm_worker_post_function_v(worker, work);
  }

  emscripten_semaphore_async_acquire(&workDone, NUM_THREADS, allThreadsDone, 0, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
}
