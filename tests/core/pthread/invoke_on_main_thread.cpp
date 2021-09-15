#include <iostream>

#include <emscripten.h>

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

      // If we are on the main thread, it is time to end this test. Do so
      // asynchronously, as if we exit right now then the object Foo() we are
      // called on will not yet be destroyed, which causes a false positive in
      // LSan leak detection.
      if (mainThread) {
        emscripten_async_call(
          [](void*) {
            exit(0);
          },
          nullptr,
          0
        );
      }
    }
  };

  // Call it on this thread.
  Foo()();

  // Call it on the main thread.
  emscripten::invoke_on_main_thread_async(Foo());

  emscripten_exit_with_live_runtime();
}
