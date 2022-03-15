#include <cassert>
#include <condition_variable>
#include <iostream>
#include <sched.h>

#include "proxying.h"

using namespace emscripten;

// The worker threads we will use. `looper` sits in a loop, continuously
// processing work as it becomes available, while `returner` returns to the JS
// event loop each time it processes work.
std::thread looper;
std::thread returner;

// The queue used to send work to both `looper` and `returner`.
ProxyingQueue queue;

// Whether `looper` should exit.
std::atomic<bool> should_quit{false};

// Whether `returner` has spun up.
std::atomic<bool> has_begun{false};

void looper_main() {
  while (!should_quit) {
    queue.execute();
    sched_yield();
  }
}

void returner_main() {
  has_begun = 1;
  emscripten_exit_with_live_runtime();
}

void test_proxy_async() {
  std::cout << "Testing async proxying\n";

  int i = 0;

  std::mutex mutex;
  std::condition_variable cond;
  std::thread::id executor;

  // Proxy to ourselves.
  queue.proxyAsync(pthread_self(), [&]() {
    i = 1;
    executor = std::this_thread::get_id();
  });
  assert(i == 0);
  queue.execute();
  assert(i == 1);
  assert(executor == std::this_thread::get_id());

  // Proxy to looper.
  {
    queue.proxyAsync(looper.native_handle(), [&]() {
      i = 2;
      executor = std::this_thread::get_id();
      cond.notify_one();
    });
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [&]() { return i == 2; });
    assert(executor == looper.get_id());
  }

  // Proxy to returner.
  {
    queue.proxyAsync(returner.native_handle(), [&]() {
      i = 3;
      executor = std::this_thread::get_id();
      cond.notify_one();
    });
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [&]() { return i == 3; });
    assert(executor == returner.get_id());
  }
}

void test_proxy_sync() {
  std::cout << "Testing sync proxying\n";

  int i = 0;
  std::thread::id executor;

  // Proxy to looper.
  {
    queue.proxySync(looper.native_handle(), [&]() {
      i = 2;
      executor = std::this_thread::get_id();
    });
    assert(i == 2);
    assert(executor == looper.get_id());
  }

  // Proxy to returner.
  {
    queue.proxySync(returner.native_handle(), [&]() {
      i = 3;
      executor = std::this_thread::get_id();
    });
    assert(i == 3);
    assert(executor == returner.get_id());
  }
}

void test_proxy_sync_with_ctx(void) {
  std::cout << "Testing sync proxying\n";

  int i = 0;
  std::thread::id executor;

  // Proxy to looper.
  {
    queue.proxySyncWithCtx(looper.native_handle(), [&](auto ctx) {
      i = 2;
      executor = std::this_thread::get_id();
      ctx.finish();
    });
    assert(i == 2);
    assert(executor == looper.get_id());
  }

  // Proxy to returner.
  {
    queue.proxySyncWithCtx(returner.native_handle(), [&](auto ctx) {
      i = 3;
      executor = std::this_thread::get_id();
      auto finish = (void(*)(void*))emscripten_proxy_finish;
      emscripten_async_call(finish, ctx.ctx, 0);
    });
    assert(i == 3);
    assert(executor == returner.get_id());
  }
}

int main(int argc, char* argv[]) {
  looper = std::thread(looper_main);
  returner = std::thread(returner_main);

  // `returner` can't process its queue until it starts up.
  while (!has_begun) {
    sched_yield();
  }

  test_proxy_async();
  test_proxy_sync();
  test_proxy_sync_with_ctx();

  should_quit = true;
  looper.join();

  pthread_cancel(returner.native_handle());
  returner.join();

  std::cout << "done\n";
}
