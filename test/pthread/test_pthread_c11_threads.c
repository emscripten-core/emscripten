#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <threads.h>

tss_t key;
cnd_t cond;
mtx_t mutex;
once_flag flag = ONCE_FLAG_INIT;

_Atomic int thread_counter = 0;
_Atomic int once_counter = 0;
_Atomic int destructor_counter = 0;

void do_once(void) {
  once_counter++;
  printf("in do_once\n");
}

// Because this thread is detached it can still be running
// when the main thread exits.  And because of an emscripten
// bug (https://github.com/emscripten-core/emscripten/issues/15186).
// this means we can't write to stdout after the main thread
// exits.  This means we can't use `thread_main` below because
// the destructor to the `tss_t key` writes to stdout.
int thread_main_detached(void* arg) {
  printf("in thread_main_detached %p\n", (void*)thrd_current());
  mtx_lock(&mutex);
  thread_counter--;
  cnd_signal(&cond);
  mtx_unlock(&mutex);
  return 0;
}

int thread_main(void* arg) {
  printf("in thread_main %p\n", (void*)thrd_current());
  tss_set(key, (void*)thrd_current());
  call_once(&flag, do_once);
  mtx_lock(&mutex);
  thread_counter--;
  cnd_signal(&cond);
  mtx_unlock(&mutex);
  return 42;
}

int run_with_exit(void* arg) {
  thrd_yield();
  thrd_exit(43);
}

void destructor(void* val) {
  printf("destructor: %p\n", (void*)thrd_current());
  assert(val == (void*)thrd_current());
  destructor_counter++;
}

int main(int argc, char* argv[]) {
  int result = 0;
  printf("thrd_current: %p\n", (void*)thrd_current());

  assert(tss_create(&key, destructor) == thrd_success);

  // Test call_once
  thrd_t t1;
  thrd_t t2;
  thrd_t t3;
  thrd_t t4;

  assert(cnd_init(&cond) == thrd_success);
  assert(mtx_init(&mutex, mtx_plain) == thrd_success);
  assert(mtx_lock(&mutex) == thrd_success);

  assert(thrd_create(&t1, thread_main, NULL) == thrd_success);
  thread_counter++;
  assert(thrd_create(&t2, thread_main, NULL) == thrd_success);
  thread_counter++;
  assert(thrd_create(&t3, thread_main, NULL) == thrd_success);
  thread_counter++;
  assert(thrd_create(&t4, thread_main, NULL) == thrd_success);
  thread_counter++;

  assert(!thrd_equal(t1, t2));
  assert(thrd_equal(t1, t1));

  while (thread_counter)
    assert(cnd_wait(&cond, &mutex) == thrd_success);

  assert(thrd_join(t1, &result) == thrd_success);
  printf("t1 joined\n");
  assert(thrd_join(t2, &result) == thrd_success);
  printf("t2 joined\n");
  assert(thrd_join(t3, &result) == thrd_success);
  printf("t3 joined\n");
  assert(thrd_join(t4, &result) == thrd_success);
  printf("t4 joined\n");
  assert(result == 42);
  assert(once_counter == 1);

  assert(destructor_counter == 4);

  // Test thrd_exit return value
  thrd_t t5;
  assert(thrd_create(&t5, run_with_exit, NULL) == thrd_success);
  assert(thrd_join(t5, &result) == thrd_success);
  assert(result == 43);

  // Test thrd_detach
  thrd_t t6;
  assert(thrd_create(&t6, thread_main_detached, NULL) == thrd_success);
  assert(thrd_detach(t6) == thrd_success);

  // Wait for the thread to at least be done printing before exiting
  // the process.
  // We shouldn't need to do this but there is a bug in emscripten
  // where a deadlock can occur between main thread calling fflush()
  // during exitRuntime and the detached thread calling print (and
  // therefore holding the stdout lock).
  // See https://github.com/emscripten-core/emscripten/issues/15186.
  assert(cnd_wait(&cond, &mutex) == thrd_success);

  printf("done!\n");
  return 0;
}
