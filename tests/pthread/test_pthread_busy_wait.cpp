#include <atomic>
#include <thread>
#include <cstdio>
#include <emscripten/threading.h>

int main() {
  std::atomic<bool> done(false);

  std::thread t([&]{
    printf("in thread\n");
    done = true;
  });

  while (!done) {
    // TODO(sbc): this call should not be needed.
    // See: https://github.com/emscripten-core/emscripten/issues/15868
    emscripten_main_thread_process_queued_calls();
    std::this_thread::yield();
  }

  t.join();
  printf("done\n");
  return 0;
}
