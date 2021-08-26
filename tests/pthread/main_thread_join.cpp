// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <pthread.h>
#include <stdio.h>

#include <atomic>

pthread_t thread;

std::atomic<int> tries;

static const int EXPECTED_TRIES = 7;

void loop() {
  void* retval;
  printf("try...\n");
  if (pthread_tryjoin_np(thread, &retval) == 0) {
    emscripten_cancel_main_loop();
    assert(tries.load() == EXPECTED_TRIES);
    emscripten_force_exit(2);
  }
  tries++;
}

void *ThreadMain(void *arg) {
#ifdef TRY_JOIN
  // Delay to force the main thread to try and fail a few times before
  // succeeding.
  while (tries.load() < EXPECTED_TRIES) {}
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

