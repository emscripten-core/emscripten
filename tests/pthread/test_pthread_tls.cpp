#include <thread>
#include <cstdio>
#include <cassert>

thread_local int tls;
thread_local struct {
  int a;
  double b;
} data = {1, 2};
thread_local int array[10];

void thread(void) {
  ++tls;
  data.a = 3;
  data.b = 4;
  assert(tls == 1);
  assert(data.a == 3);
  assert(data.b == 4);
  assert(array[9] == 0);
}

int main(void) {
  array[9] = 1337;
  std::thread t(thread);
  t.join();
  assert(tls == 0);
  assert(data.a == 1);
  assert(data.b == 2);
  assert(array[9] == 1337);
  return 0;
}
