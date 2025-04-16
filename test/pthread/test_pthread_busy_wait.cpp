#include <atomic>
#include <thread>
#include <cstdio>

int main() {
  std::atomic<bool> done(false);

  std::thread t([&]{
    printf("in thread\n");
    done = true;
  });

  while (!done) {
    std::this_thread::yield();
  }

  t.join();
  printf("done\n");
  return 0;
}
