#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <assert.h>

double performanceNow;
double dateNow;

void test(void *userData) {
  double now2 = emscripten_performance_now();
  assert(now2 >= performanceNow + 100);

  double now3 = emscripten_date_now();
  assert(now3 >= dateNow + 100);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main() {
  performanceNow = emscripten_performance_now();
  assert(performanceNow < 10*1000); // Should take well less than 10 seconds to load up the page

  dateNow = emscripten_date_now();
  assert(dateNow > 1547568082); // == 2019-01-15T16:01:22+00:00)

#ifdef __EMSCRIPTEN_PTHREADS__
  emscripten_thread_sleep(200);
  test(0);
#else
  emscripten_set_timeout(test, 200, 0);
#endif
}
