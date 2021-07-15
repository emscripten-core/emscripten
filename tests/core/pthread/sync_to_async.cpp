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

  // We need to be very careful about the lifetime of |resume| below, and the
  // callback we construct from it, |asyncFunc| - things whose lifetime is that
  // of the outer lambda will not live long enough for the async callback. For
  // that reason, define asyncFunc on our stack here, which will definitely
  // remain valid, since we wait synchronously for the work to be done on the
  // thread.
  sync_to_async.invoke([](emscripten::sync_to_async::Callback resume) {
    std::cout << "  Hello from sync C++ before the async\n";
    // Set up async JS, just to prove an async JS callback happens before the
    // async C++.
    EM_ASM({
      setTimeout(function() {
        console.log("  Hello from async JS");
      }, 0);
    });
    // Set up async C++, and also call resume after running it.
    emscripten_async_call([](void* arg) {
      auto resume = (emscripten::sync_to_async::Callback)arg;
      std::cout << "  Hello from async C++\n";
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
