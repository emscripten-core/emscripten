#include <cstdio>
#include <thread>

std::atomic<bool> thread_done;

void f(int *a) {
  delete [] a;
  a[0] = 1;
  std::atomic_store(&thread_done, true);
}

int main(int argc, char **argv) {
  std::thread t(f, new int[10]);
  t.detach();
  while (!std::atomic_load(&thread_done));
}
