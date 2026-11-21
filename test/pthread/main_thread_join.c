// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <pthread.h>
#include <stdio.h>

pthread_t thread;

_Atomic int tries;

// The number of times that the main thread has to try and fail to join the
// thread while the thread is running.
const int MIN_TRIES = 7;

void loop() {
  void* retval;
  printf("try...\n");
  if (pthread_tryjoin_np(thread, &retval) == 0) {
    emscripten_cancel_main_loop();
    // The thread exits only after it has seen the main thread fail to join it
    // MIN_TRIES times. Do not expect an exact count here: how many more times
    // the main thread tries before the thread starts to run (without a thread
    // pool that includes loading a Worker), and before the exit of the thread
    // takes effect, depends on timing.
    assert(tries >= MIN_TRIES);
    emscripten_force_exit(2);
  }
  tries++;
}

void *ThreadMain(void *arg) {
#ifdef TRY_JOIN
  // Delay to force the main thread to try and fail a few times before
  // succeeding. Count the tries from when this thread starts to run, since
  // the main thread may have tried several times before that already.
  int target = tries + MIN_TRIES;
  while (tries < target) {}
#endif
  pthread_exit((void*)0);
}

pthread_t CreateThread() {
  pthread_t ret;
  int rc = pthread_create(&ret, NULL, ThreadMain, (void*)0);
  assert(rc == 0);
  return ret;
}

int main() {
  thread = CreateThread();
#ifdef TRY_JOIN
  emscripten_set_main_loop(loop, 0, 0);
#else
  int status;
  // This should fail on the main thread.
  puts("trying to block...");
  pthread_join(thread, (void**)&status);
  puts("blocked ok.");
  return 0;
#endif // TRY_JOIN
}

