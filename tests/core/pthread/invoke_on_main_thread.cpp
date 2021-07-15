#include <iostream>

#include <emscripten/thread_utils.h>

int main() {
  struct Foo {
    void operator()() {
      // Print whether we are on the main thread.
      auto mainThread = EM_ASM_INT({
        var mainThread = typeof importScripts === 'undefined';
        console.log("hello. mainThread=", mainThread);
        return mainThread;
      });
      // If we are on the main thread, it is time to end this test.
      if (mainThread) exit(0);
    }
  };

  // Call it on this thread.
  Foo()();

  // Call it on the main thread.
  emscripten::invoke_on_main_thread_async(Foo());

  emscripten_exit_with_live_runtime();
}
