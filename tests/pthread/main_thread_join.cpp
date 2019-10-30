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
#ifdef REPORT_RESULT
    REPORT_RESULT(2);
#endif
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
  if (!emscripten_has_threading_support()) {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  int x = EM_ASM_INT({
    onerror = function(e) {
      var message = e.toString();
      var success = message.indexOf("Blocking on the main thread is not allowed by default. See https://emscripten.org/docs/porting/pthreads.html#blocking-on-the-main-browser-thread") >= 0;
      if (success && !Module.reported) {
        Module.reported = true;
        console.log("reporting success");
        // manually REPORT_RESULT; we shouldn't call back into native code at this point
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "http://localhost:8888/report_result?0");
        xhr.send();
      }
    };
    return 0;
  });

  thread = CreateThread();
#ifdef TRY_JOIN
  emscripten_set_main_loop(loop, 0, 0);
#else
  int status;
  // This should fail on the main thread.
  puts("trying to block...");
  pthread_join(thread, (void**)&status);
  puts("blocked ok.");
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
#endif // TRY_JOIN
}

