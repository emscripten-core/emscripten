#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <emscripten.h>
#include <sanitizer/lsan_interface.h>

void *global_ptr;
thread_local void *tls_ptr;
std::atomic<bool> thread_done;

void f(void) {
  void *local_ptr = malloc(42);
  memset(local_ptr, 0, 42);
}

void g(void) {
  void *stuff = malloc(3432);
  stuff = 0;
  tls_ptr = malloc(1234);
  tls_ptr = 0;
  atomic_store(&thread_done, true);
  while (1);
}

int main(int argc, char **argv) {
  std::thread t(g);
  t.detach();

  global_ptr = malloc(1337);
  tls_ptr = malloc(420);
  tls_ptr = 0;
  memset(global_ptr, 0, 42);
  global_ptr = 0;
  f();
  memset(calloc(16, 128), 48, 2048);
  while (!atomic_load(&thread_done));

  __lsan_do_leak_check();
  fprintf(stderr, "LSAN TEST COMPLETE\n");
  return 0;
}
