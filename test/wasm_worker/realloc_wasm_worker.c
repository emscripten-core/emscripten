#include <assert.h>
#include <emscripten.h>
#include <emscripten/em_math.h>
#include <emscripten/wasm_worker.h>
#include <string.h>

#define NUM_THREADS emscripten_navigator_hardware_concurrency()

// Stress test realloc() behavior after a suspected instability of realloc()
// function in https://groups.google.com/g/emscripten-discuss/c/xvcjByZKxVA

int mempatterncmp(unsigned char* ptr, size_t size, unsigned char byte) {
  for (size_t i = 0; i < size; ++i)
    if (ptr[i] != byte)
      return 1;
  return 0;
}

emscripten_semaphore_t work_done = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(0);

extern int emmalloc_validate_memory_regions(void);

// Each worker realloc()s increasing amounts of memory in a loop.
void worker_main() {
  emscripten_console_log("Wasm worker starting realloc() stress test");
  unsigned char byteFillPattern =
    (unsigned char)(emscripten_math_random() * 0xFF);
  unsigned char* ptr = 0;
  size_t size = 0;
#define ALLOC_SZ 128 * 1024
  // Stress test realloc() in a loop
  size_t maxAllocSize = ALLOC_SZ;
  for (int i = 0; i < 128; ++i) {
    size_t newSize = emscripten_math_random() * maxAllocSize;
    maxAllocSize += ALLOC_SZ;
    emmalloc_validate_memory_regions();
    unsigned char* newPtr = realloc(ptr, newSize);
    emmalloc_validate_memory_regions();
    size_t oldSizeLeft = (size < newSize) ? size : newSize;
    if (newPtr || newSize == 0) {
      ptr = newPtr;
      size = newSize;
    }
    // Verify that after realloc, the new pointer should hold the old byte fill
    // pattern
    emmalloc_validate_memory_regions();
    assert(!mempatterncmp(ptr, oldSizeLeft, byteFillPattern));
    emmalloc_validate_memory_regions();

    // Fill the newly allocated memory with a new byte fill pattern
    ++byteFillPattern;
    //  EM_ASM(console.log('Worker ' + Module['$ww'] + ': doing memory fill of
    //  address 0x'+$0.toString(16) + ' size ' + $1 + ' with byte
    //  0x'+$2.toString(16)), ptr, size, byteFillPattern);
    emmalloc_validate_memory_regions();
    memset(ptr, byteFillPattern, size);
    emmalloc_validate_memory_regions();
  }
  emscripten_console_log("Wasm worker finished realloc() stress test");
  emscripten_semaphore_release(&work_done, 1);
}

void test_done_listener() {
  emscripten_semaphore_waitinf_acquire(&work_done, NUM_THREADS);
  emscripten_console_log(
    "All threads successfully finished stress testing realloc()!");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main() {
  for (int i = 0; i < NUM_THREADS; ++i) {
    emscripten_wasm_worker_t worker =
      emscripten_malloc_wasm_worker(/*stack size: */ 1024);
    emscripten_wasm_worker_post_function_v(worker, worker_main);
  }
  emscripten_wasm_worker_t listener =
    emscripten_malloc_wasm_worker(/*stack size: */ 1024);
  emscripten_wasm_worker_post_function_v(listener, test_done_listener);
}
