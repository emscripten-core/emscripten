#include <thread>
#include <cstdio>

thread_local int tls;

void thread(void) {
  ++tls;
  assert(tls == 1);
}

int main(void) {
  std::thread t(thread);
  t.join();
  assert(tls == 0);
#ifdef REPORT_RESULT
  REPORT_RESULT(1337);
#endif
}
