#include <stdio.h>
#include <emscripten.h>
#ifdef __EMSCRIPTEN_PTHREADS__
#include <thread>
#endif

extern "C" EMSCRIPTEN_KEEPALIVE void foo() {
  printf("foo\n");
}

extern "C" void bar() {
  printf("bar\n");
}

int main() {
  printf("main1\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  std::thread([]{
    printf("main2\n");
  }).join();
#else
  printf("main2\n");
#endif
}
