#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/stack.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define THREAD_STACK_SIZE 1024
#define NUM_THREADS 2
void *thread_stack[NUM_THREADS];

volatile int threadsOk = 0;

void test_stack(int i)
{
  EM_ASM(console.log(`In thread ${$0}, stack low addr=0x${$1.toString(16)}, emscripten_stack_get_base()=0x${$2.toString(16)}, emscripten_stack_get_end()=0x${$3.toString(16)}, THREAD_STACK_SIZE=0x${$4.toString(16)}`),
    i, thread_stack[i], emscripten_stack_get_base(), emscripten_stack_get_end(), THREAD_STACK_SIZE);
  assert(emscripten_stack_get_base() == (uintptr_t)thread_stack[i] + THREAD_STACK_SIZE);
  assert(emscripten_stack_get_end() == (uintptr_t)thread_stack[i]);

  int ok = __sync_fetch_and_add(&threadsOk, 1);
  EM_ASM(console.log($0), ok);
  if (ok == 1)
  {
    EM_ASM(console.log(`Test finished!`));
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
  }
}

int main()
{
  EM_ASM(console.log(`Main thread stack base=0x${$0.toString(16)}, end=0x${$1.toString(16)}`), emscripten_stack_get_base(), emscripten_stack_get_end());

  for(int i = 0; i < NUM_THREADS; ++i)
  {
    thread_stack[i] = memalign(16, THREAD_STACK_SIZE);
    emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(thread_stack[i], THREAD_STACK_SIZE);
    EM_ASM(console.log(`Created thread ${$0} with stack ptr=0x${$1.toString(16)}, size=0x${$2.toString(16)}`), i, thread_stack[i], THREAD_STACK_SIZE);
    emscripten_wasm_worker_post_function_vi(worker, test_stack, i);
  }
}
