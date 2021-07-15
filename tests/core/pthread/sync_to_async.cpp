#include <assert.h>
#include <emscripten.h>
#include <pthread.h>

#include <thread>
#include <functional>
#include <iostream>

#include <emscripten/thread_utils.h>

int main() {
  SyncToAsync helper;

  std::cout << "Perform a synchronous task.\n";

  helper.doWork([](SyncToAsync::Callback resume) {
    std::cout << "  Hello from sync C++\n";
    resume();
  });

  std::cout << "Perform an async task.\n";

  // We need to be very careful about the lifetime of |resume| below, and the
  // callback we construct from it, |asyncFunc| - things whose lifetime is that
  // of the outer lambda will not live long enough for the async callback. For
  // that reason, define asyncFunc on our stack here, which will definitely
  // remain valid, since we wait synchronously for the work to be done on the
  // thread.
  SyncToAsync::Callback asyncFunc;
  helper.doWork([&asyncFunc](SyncToAsync::Callback resume) {
    std::cout << "  Hello from sync C++ before the async\n";
    // Set up async JS, just to prove an async JS callback happens before the
    // async C++.
    EM_ASM({
      setTimeout(function() {
        console.log("  Hello from async JS");
      }, 0);
    });
    // Set up async C++.
    asyncFunc = [resume]() {
      std::cout << "  Hello from async C++\n";
      resume();
    };
    emscripten_async_call([](void* arg) {
      auto* funcAddr = (SyncToAsync::Callback*)arg;
      (*funcAddr)();
    }, &asyncFunc, 1);
  });

  std::cout << "Perform another synchronous task.\n";
  helper.doWork([](SyncToAsync::Callback resume) {
    std::cout << "  Hello again from sync C++\n";
    resume();
  });
  return 0;
}
