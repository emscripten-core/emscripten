#include <emscripten.h>
#include <pthread.h>
#include <assert.h>

// Tests behavior of __proxy: 'abort' and __proxy: 'abort_debug' in pthreads.

void proxied_js_function(void);

int might_throw(void(*func)()) {
  int threw = EM_ASM_INT({
    // Patch over assert() so that it does not abort execution on assert failure, but instead
    // throws a catchable exception.
    assert = function(condition, text) {
      if (!condition) {
        throw 'Assertion failed' + (text ? ": " + text : "");
      }
    };

    try {
      getWasmTableEntry($0)();
    } catch(e) {
      console.error('Threw an exception: ' + e);
      return e.toString().includes('this function has been declared to only be callable from the main browser thread');
    }
    console.error('Function did not throw');
    return 0;
  }, func);
  return threw;
}

void test() {
  proxied_js_function();
}

void *thread_main(void *arg) {
  REPORT_RESULT(might_throw(test));
  return 0;
}

char stack[1024];

int main() {
  proxied_js_function(); // Should be callable from main thread

  pthread_t thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&thread, &attr, thread_main, 0);
}
