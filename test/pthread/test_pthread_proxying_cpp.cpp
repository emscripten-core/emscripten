#include <cassert>
#include <condition_variable>
#include <emscripten/proxying.h>
#include <emscripten/eventloop.h>
#include <iostream>
#include <sched.h>

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

void looper_main() {
  while (!should_quit) {
    queue.execute();
    sched_yield();
  }
}

void returner_main() {
  // Return back to the event loop while keeping the runtime alive.
  // Note that we can't use `emscripten_exit_with_live_runtime` here without
  // introducing a memory leak due to way to C++11 threads interact with
  // unwinding. See https://github.com/emscripten-core/emscripten/issues/17091.
  emscripten_runtime_keepalive_push();
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
      {
        std::unique_lock<std::mutex> lock(mutex);
        i = 2;
      }
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
      {
        std::unique_lock<std::mutex> lock(mutex);
        i = 3;
      }
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
  std::cout << "Testing sync_with_ctx proxying\n";

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

void test_proxy_callback(void) {
  std::cout << "Testing callback proxying\n";

  int i = 0;
  thread_local int j = 0;
  std::thread::id executor;

  // Proxy to ourselves.
  queue.proxyCallback(
    pthread_self(),
    [&]() {
      i = 1;
      executor = std::this_thread::get_id();
    },
    [&]() { j = 1; },
    {});
  assert(i == 0);
  queue.execute();
  assert(i == 1);
  assert(executor == std::this_thread::get_id());
  assert(j == 1);

  // Proxy to looper.
  {
    queue.proxyCallback(
      looper.native_handle(),
      [&]() {
        i = 2;
        executor = std::this_thread::get_id();
      },
      [&]() { j = 2; },
      {});
    // TODO: Add a way to wait for work before executing it.
    while (j != 2) {
      queue.execute();
    }
    assert(i == 2);
    assert(executor == looper.get_id());
  }

  // Proxy to returner.
  {
    queue.proxyCallback(
      returner.native_handle(),
      [&]() {
        i = 3;
        executor = std::this_thread::get_id();
      },
      [&]() { j = 3; },
      {});
    // TODO: Add a way to wait for work before executing it.
    while (j != 3) {
      queue.execute();
    }
    assert(i == 3);
    assert(executor == returner.get_id());
  }
}

void test_proxy_callback_with_ctx(void) {
  std::cout << "Testing callback_with_ctx proxying\n";

  int i = 0;
  thread_local int j = 0;
  std::thread::id executor;

  // Proxy to ourselves.
  queue.proxyCallbackWithCtx(
    pthread_self(),
    [&](auto ctx) {
      i = 1;
      executor = std::this_thread::get_id();
      ctx.finish();
    },
    [&]() { j = 1; },
    {});
  assert(i == 0);
  queue.execute();
  assert(i == 1);
  assert(executor == std::this_thread::get_id());
  assert(j == 1);

  // Proxy to looper.
  {
    queue.proxyCallbackWithCtx(
      looper.native_handle(),
      [&](auto ctx) {
        i = 2;
        executor = std::this_thread::get_id();
        ctx.finish();
      },
      [&]() { j = 2; },
      {});
    // TODO: Add a way to wait for work before executing it.
    while (j != 2) {
      queue.execute();
    }
    assert(i == 2);
    assert(executor == looper.get_id());
  }

  // Proxy to returner.
  {
    queue.proxyCallbackWithCtx(
      returner.native_handle(),
      [&](auto ctx) {
        i = 3;
        executor = std::this_thread::get_id();
        auto finish = (void (*)(void*))emscripten_proxy_finish;
        emscripten_async_call(finish, ctx.ctx, 0);
      },
      [&]() { j = 3; },
      {});
    // TODO: Add a way to wait for work before executing it.
    while (j != 3) {
      queue.execute();
    }
    assert(i == 3);
    assert(executor == returner.get_id());
  }
}

int main(int argc, char* argv[]) {
  looper = std::thread(looper_main);
  returner = std::thread(returner_main);

  test_proxy_async();
  test_proxy_sync();
  test_proxy_sync_with_ctx();
  test_proxy_callback();
  test_proxy_callback_with_ctx();

  should_quit = true;
  looper.join();

  pthread_cancel(returner.native_handle());
  returner.join();

  std::cout << "done\n";
}
