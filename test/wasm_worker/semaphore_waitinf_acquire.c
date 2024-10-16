#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <assert.h>

// Tests emscripten_semaphore_init(), emscripten_semaphore_waitinf_acquire() and emscripten_semaphore_release()

emscripten_semaphore_t threadsWaiting = (emscripten_semaphore_t)12345315; // initialize with garbage
emscripten_semaphore_t threadsRunning = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(0); // initialize with static initializer
emscripten_semaphore_t threadsCompleted = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(0);

int threadCounter = 0;

void worker_main() {
  emscripten_console_log("worker_main");

  // Increment semaphore to mark that this thread is waiting for a signal from control thread to start.
  emscripten_semaphore_release(&threadsWaiting, 1);

  // Acquire thread run semaphore once main thread has given this thread a go signal.
  emscripten_console_log("worker_main: waiting for thread run signal");
  emscripten_semaphore_waitinf_acquire(&threadsRunning, 1);

  // Do heavy computation:
  emscripten_console_log("worker_main: incrementing work");
  emscripten_atomic_add_u32((void*)&threadCounter, 1);

  // Increment semaphore to signal that this thread has finished.
  emscripten_console_log("worker_main: thread completed");
  emscripten_semaphore_release(&threadsCompleted, 1);
}

void control_thread() {
  // Wait until we have three threads available to start running.
  emscripten_console_log("control_thread: waiting for three threads to complete loading");
  emscripten_semaphore_waitinf_acquire(&threadsWaiting, 3);

  // Set the three waiting threads to run simultaneously.
  assert(threadCounter == 0);
  emscripten_console_log("control_thread: release three threads to run");
  emscripten_semaphore_release(&threadsRunning, 3);

  // Wait until we have 3 threads completed their run.
  emscripten_console_log("control_thread: waiting for three threads to complete");
  emscripten_semaphore_waitinf_acquire(&threadsCompleted, 3);
  assert(threadCounter == 3);

  // Wait until we have next 3 threads available to start running.
  emscripten_console_log("control_thread: waiting for next three threads to be ready");
  emscripten_semaphore_waitinf_acquire(&threadsWaiting, 3);

  // Set the three waiting threads to run simultaneously.
  assert(threadCounter == 3);
  emscripten_console_log("control_thread: setting next three threads go");
  emscripten_semaphore_release(&threadsRunning, 3);

  // Wait until we have the final 3 threads completed their run.
  emscripten_console_log("control_thread: waiting for the last three threads to finish");
  emscripten_semaphore_waitinf_acquire(&threadsCompleted, 3);
  emscripten_console_log("control_thread: threads finished");
  assert(threadCounter == 6);

  emscripten_console_log("control_thread: test finished");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main() {
  emscripten_semaphore_init(&threadsWaiting, 0);

  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024);
  emscripten_wasm_worker_post_function_v(worker, control_thread);

#define NUM_THREADS 6
  for (int i = 0; i < NUM_THREADS; ++i) {
    emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024);
    emscripten_wasm_worker_post_function_v(worker, worker_main);
  }
}
