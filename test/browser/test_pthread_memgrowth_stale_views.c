#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

intptr_t test_addr = 0;

void* thread_run(void* arg) {
  // Query the JS HEAP8 view size immediately.
  intptr_t heap_len = (intptr_t)EM_ASM_PTR({ return HEAP8.byteLength; });

  printf("thread: HEAP8.len=%zu, test_addr=%zu\n", heap_len, test_addr);

  // Verify that the JS view has been updated to cover the new allocation.
  // If the view is stale, heap_len will be smaller than test_addr.
  assert(heap_len >= test_addr && "test failed: JS views are stale.");

  printf("test succeeded.\n");
  emscripten_force_exit(0);
  return NULL;
}

int main() {
  printf("main start\n");
  pthread_t thread;
  pthread_create(&thread, NULL, thread_run, NULL);

  printf("main allocating 20MB\n");
  void* dummy_ptr = malloc(20 * 1024 * 1024);
  test_addr = (intptr_t)dummy_ptr + 19 * 1024 * 1024;
  printf("main allocated\n");

  printf("main exiting with live runtime\n");
  emscripten_exit_with_live_runtime();
}
