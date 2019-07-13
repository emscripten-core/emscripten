#include <thread>
#include <cstdio>

thread_local int tls = 1337;

int main(void) {
#ifdef REPORT_RESULT
  REPORT_RESULT(tls);
#endif
}
