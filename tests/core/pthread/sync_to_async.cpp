#include <iostream>

#include <emscripten/thread_utils.h>

int main() {
  emscripten::sync_to_async sync_to_async;

  std::cout << "Perform a synchronous task.\n";

  sync_to_async.invoke([](emscripten::sync_to_async::Callback resume) {
    std::cout << "  Hello from sync C++\n";
    (*resume)();
  });

  std::cout << "Perform an async task.\n";

  sync_to_async.invoke([](emscripten::sync_to_async::Callback resume) {
    std::cout << "  Hello from sync C++ before the async\n";

    // Set up async JS, just to prove an async JS callback happens before the
    // async C++.
    EM_ASM({
      setTimeout(function() {
        console.log("  Hello from async JS");
      }, 0);
    });

    // Set up async C++..
    emscripten_async_call([](void* arg) {
      auto resume = (emscripten::sync_to_async::Callback)arg;
      std::cout << "  Hello from async C++\n";

      // We are done with all the async things we want to do, and can call
      // resume to continue execution on the calling thread.
      (*resume)();
    }, resume, 1);
  });

  std::cout << "Perform another synchronous task.\n";

  sync_to_async.invoke([](emscripten::sync_to_async::Callback resume) {
    std::cout << "  Hello again from sync C++\n";
    (*resume)();
  });
  return 0;
}
