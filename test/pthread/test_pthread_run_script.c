#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>

EMSCRIPTEN_KEEPALIVE void FinishTest(int result) {
  printf("Test finished, result: %d\n", result);
  assert(result == 1);
  exit(0);
}

void TestAsyncRunScript() {
  // 5. Test emscripten_async_run_script() runs in a pthread.
#if __EMSCRIPTEN_PTHREADS__
  emscripten_async_run_script("Module['_FinishTest'](ENVIRONMENT_IS_PTHREAD && (typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER));", 1);
#else
  emscripten_async_run_script("Module['_FinishTest'](!(typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER));", 1);
#endif
}

void AsyncScriptLoaded() {
  printf("async script load succeeded!\n");
  TestAsyncRunScript();
}

void AsyncScriptFailed() {
  printf("async script load failed!\n");
  TestAsyncRunScript();
}

int main() {
  // 1. Test that emscripten_run_script() works in a pthread, and it gets
  // executed in the web worker and not on the main thread.
#if __EMSCRIPTEN_PTHREADS__
  emscripten_run_script("Module['ranScript'] = ENVIRONMENT_IS_PTHREAD && (typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER);");
#else
  emscripten_run_script("Module['ranScript'] = true;");
#endif

  // 2. Test that emscripten_run_script_int() works in a pthread and it gets
  // executed in the web worker and not on the main thread.
#if __EMSCRIPTEN_PTHREADS__
  int result = emscripten_run_script_int("Module['ranScript'] && ENVIRONMENT_IS_PTHREAD && (typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER);");
#else
  int result = emscripten_run_script_int("Module['ranScript'];");
#endif
  printf("Module['ranScript'] = %d\n", result);
  assert(result);

  // 3. Test emscripten_run_script_string() runs in a pthread.
#if __EMSCRIPTEN_PTHREADS__
  char *data = emscripten_run_script_string("ENVIRONMENT_IS_PTHREAD && (typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER) ? 'in pthread' : 'not in pthread';");
  printf("%s\n", data);
  assert(!strcmp(data, "in pthread"));
#else
  char *data = emscripten_run_script_string("(typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER) ? 'in worker' : 'not in worker';");
  printf("%s\n", data);
  assert(!strcmp(data, "not in worker"));
#endif

  // 4. Test emscripten_async_load_script() runs in a pthread.
  emscripten_async_load_script("foo.js", AsyncScriptLoaded, AsyncScriptFailed);

  // This return code should be ignored since emscripten_async_load_script
  // keeps the runtime alive until FinishTest is called.
  return 99;
}
