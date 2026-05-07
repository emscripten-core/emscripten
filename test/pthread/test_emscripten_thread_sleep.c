#include <assert.h>
#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdio.h>

void do_sleep(double msecs) {
  double t1 = emscripten_get_now();
  emscripten_thread_sleep(msecs);
  double t2 = emscripten_get_now();
  emscripten_outf("emscripten_thread_sleep() slept for %f msecs.\n", t2 - t1);

  assert(t2 - t1 >= 0.9 * msecs); // Should have slept ~ the requested time.
}

void* thread_main(void* arg) {
  emscripten_out("hello from thread!");

  do_sleep(1);
  do_sleep(10);
  do_sleep(100);
  do_sleep(1000);
  do_sleep(5000);

  emscripten_force_exit(0);
  return NULL;
}

int main() {
  // Bad bad bad to sleep on the main thread, but test that it works.
  do_sleep(1);
  do_sleep(10);
  do_sleep(100);
  do_sleep(1000);
  do_sleep(5000);
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  emscripten_exit_with_live_runtime();
  __builtin_trap();
}
