#include <stdio.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>

atomic_bool work_done = false;

void work() {
  // This `work` item is called from within
  // `emscripten_current_thread_process_queued_calls`.
  // Calling `emscripten_thread_sleep` here will trigger another nested call to
  // `emscripten_current_thread_process_queued_calls`.
  printf("Begin work\n");
  emscripten_thread_sleep(1);
  work_done = true;
  printf("End work\n");
}

void* thread_func(void* _param) {
  printf("Start thread\n");
  while (!work_done) {
    emscripten_thread_sleep(1);
  }
  printf("End thread\n");
  return NULL;
};

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, thread_func, (void *)0);

  emscripten_dispatch_to_thread(thread, EM_FUNC_SIG_V, &work, 0);

  pthread_join(thread, NULL);
  printf("Test finish\n");
}
