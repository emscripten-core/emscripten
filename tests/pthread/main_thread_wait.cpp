// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <pthread.h>
#include <stdio.h>

int main() {
  if (!emscripten_has_threading_support())
  {
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

  // This should fail on the main thread.
  puts("trying to block...");
  pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_wait(&cv, &lock);
  puts("blocked ok.");
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}

