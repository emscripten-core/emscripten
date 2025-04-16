#include <stdlib.h>
#include <pthread.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#error "should be compiled without threads"
#endif

void* run(void* arg) {
  return arg;
}

int main(int argc, char **argv) {
  pthread_t thread;
  int rtn = pthread_create(&thread, NULL, run, 0);
  // Should fail since threads were not compiled in.
  return rtn == 0 ? 1 : 0;
}
