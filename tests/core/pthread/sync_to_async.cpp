#include <assert.h>
#include <emscripten.h>
#include <pthread.h>

#include <thread>
#include <functional>
#include <iostream>

// Helper class for generic sync-to-async conversion. doWork() lets you provide
// some code to be run, which is done on a helper thread. The thread can do
// async operations, and it has a callback to indicate when it finishes working.
// The calling thread mainwhile blocks as it waits.
//
// This can be useful if you are a location where blocking is possible (like a
// thread, or when using PROXY_TO_PTHREAD), but you have code that is hard to
// refactor to be async, but that requires some async operation (like waiting
// for a JS event).
class SyncToAsync {
public:
  using Callback = std::function<void()>;

  SyncToAsync() : thread(threadMain, this), childLock(mutex) {
    // The child lock is associated with the mutex, which takes the lock, and
    // we free it here. Only the child will lock/unlock it from now on.
    childLock.unlock();
  }

  ~SyncToAsync() {
    quit = true;

    // Wake up the child with an empty task.
    doWork([](Callback func){
      func();
    });

    thread.join();
  }

  // Run some work on thread. This is a synchronous call, but the thread can do
  // async work for us. To allow us to know when the async work finishes, the
  // worker is given a function to call at that time.
  void doWork(std::function<void(Callback)> newWork) {
    // Send the work over.
    {
      std::lock_guard<std::mutex> lock(mutex);
      work = newWork;
      finishedWork = false;
      readyToWork = true;
    }
    condition.notify_one();

    // Wait for it to be complete.
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [&]() {
      return finishedWork;
    });
  }

private:
  std::thread thread;
  std::mutex mutex;
  std::condition_variable condition;
  std::function<void(Callback)> work;
  bool readyToWork = false;
  bool finishedWork;
  bool quit = false;

  // The child will be asynchronous, and therefore we cannot rely on RAII to
  // unlock for us, we must do it manually.
  std::unique_lock<std::mutex> childLock;

  static void* threadMain(void* arg) {
    auto* parent = (SyncToAsync*)arg;
    emscripten_async_call(threadIter, arg, 0);
    return 0;
  }

  static void threadIter(void* arg) {
    auto* parent = (SyncToAsync*)arg;
    // Wait until we get something to do.
    parent->childLock.lock();
    parent->condition.wait(parent->childLock, [&]() {
      return parent->readyToWork;
    });
    auto work = parent->work;
    parent->readyToWork = false;
    // Do the work.
    work([parent, arg]() {
      // We are called, so the work was finished. Notify the caller.
      parent->finishedWork = true;
      parent->childLock.unlock();
      parent->condition.notify_one();
      if (parent->quit) {
        // This works fine although it is a c++11 std::thread.
        pthread_exit(0);
      } else {
        // Look for more work. (We do this asynchronously to avoid nesting of
        // the stack, and to keep this function simple without a loop.)
        emscripten_async_call(threadIter, arg, 0);
      }
    });
  }
};

// Testcase

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
