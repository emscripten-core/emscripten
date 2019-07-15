#include <thread>
#include <cstdio>

thread_local int tls = 1330;
thread_local int tls2;

__attribute__((constructor))
void init_tls2(void) {
  tls2 = 7;
}

int main(void) {
#ifdef REPORT_RESULT
  REPORT_RESULT(tls + tls2);
#endif
}
