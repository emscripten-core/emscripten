#include <thread>

int main() {
  std::thread t([]{
    printf("in thread\n");
  });
  t.join();
  printf("done\n");
  return 0;
}
