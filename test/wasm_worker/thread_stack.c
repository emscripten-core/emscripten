#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/stack.h>
#include <emscripten/console.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define ALIGN(x,y) ((x)+(y)-1 & -(y))
#define THREAD_STACK_SIZE 2048
#define NUM_THREADS 2
void *thread_stack[NUM_THREADS];

volatile int threadsOk = 0;

void test_stack(int i) {
  emscripten_outf("In thread %d, stack low addr=%p, emscripten_stack_get_base()=%p, emscripten_stack_get_end()=%p, THREAD_STACK_SIZE=%d",
    i, thread_stack[i],
    (void*)emscripten_stack_get_base(),
    (void*)emscripten_stack_get_end(),
    THREAD_STACK_SIZE);
  assert(emscripten_stack_get_base() == (uintptr_t)thread_stack[i] + THREAD_STACK_SIZE);
  emscripten_outf("__builtin_wasm_tls_size: %lu", __builtin_wasm_tls_size());
  // The stack region in wasm workers also incldues TLS, so we need to take
  // this into account when calulating our expected stack end.
  size_t expected_stack_end = (uintptr_t)thread_stack[i] + ALIGN(__builtin_wasm_tls_size(), 16);
  assert(emscripten_stack_get_end() == expected_stack_end);

  int ok = __sync_fetch_and_add(&threadsOk, 1);
  emscripten_outf("%d", ok);
  if (ok == 1) {
    emscripten_out("Test finished!");
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
  }
}

int main() {
  emscripten_outf("Main thread stack base=%p, end=%p", (void*)emscripten_stack_get_base(), (void*)emscripten_stack_get_end());

  for (int i = 0; i < NUM_THREADS; ++i) {
    thread_stack[i] = memalign(16, THREAD_STACK_SIZE);
    emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(thread_stack[i], THREAD_STACK_SIZE);
    emscripten_outf("Created thread %d with stack ptr=%p, end=%p, size=%x", i, thread_stack[i], thread_stack[i] + THREAD_STACK_SIZE, THREAD_STACK_SIZE);
    emscripten_wasm_worker_post_function_vi(worker, test_stack, i);
  }
}
