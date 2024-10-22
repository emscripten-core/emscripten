#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test the function emscripten_wasm_worker_self_id()

emscripten_wasm_worker_t worker1 = 0;
emscripten_wasm_worker_t worker2 = 0;

int successes = 0;

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void test_success() {
  if (__atomic_add_fetch(&successes, 1, __ATOMIC_SEQ_CST) == 2) {
    emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
  }
}

void worker1_main() {
  assert(emscripten_wasm_worker_self_id() != 0);
  assert(emscripten_wasm_worker_self_id() == worker1);
  if (emscripten_wasm_worker_self_id() == worker1) {
    test_success();
  }
}

void worker2_main() {
  assert(emscripten_wasm_worker_self_id() != 0);
  assert(emscripten_wasm_worker_self_id() == worker2);
  if (emscripten_wasm_worker_self_id() == worker2) {
    test_success();
  }
}

char stack1[1024];
char stack2[1024];

int main() {
  assert(emscripten_wasm_worker_self_id() == 0);
  worker1 = emscripten_create_wasm_worker(stack1, sizeof(stack1));
  worker2 = emscripten_create_wasm_worker(stack2, sizeof(stack2));
  emscripten_wasm_worker_post_function_v(worker1, worker1_main);
  emscripten_wasm_worker_post_function_v(worker2, worker2_main);
  emscripten_exit_with_live_runtime();
}
