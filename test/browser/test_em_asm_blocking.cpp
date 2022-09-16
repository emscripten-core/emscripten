#include <assert.h>
#include <emscripten/em_asm.h>
#include <stdio.h>
#include <thread>
#include <math.h>

std::atomic<int> ret;

void foo() {
  int len = MAIN_THREAD_EM_ASM_INT({
    var elem = document.getElementById('elem');
    window.almost_PI = 3.14159;
    return elem.innerText.length;
  });
  double almost_PI = MAIN_THREAD_EM_ASM_DOUBLE({
    // read a double from the main thread
    return window.almost_PI;
  });
  printf("almost PI: %f\n", almost_PI);
  assert(fabs(almost_PI - 3.14159) < 0.001);
  ret = len;
}

int main() {
  std::thread t(foo);
  t.join();
  printf("ret: %d\n", ret.load());
  return ret.load();
}
