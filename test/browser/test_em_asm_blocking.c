#include <assert.h>
#include <emscripten/em_asm.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

_Atomic int ret;

void* foo(void* arg) {
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
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, foo, NULL);
  pthread_join(t, NULL);
  printf("ret: %d\n", ret);
  assert(ret == 8);
  return 0;
}
