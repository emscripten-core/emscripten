#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <emscripten.h>

void *global_ptr;
thread_local void *tls_ptr;
std::atomic<bool> thread_done;

void g(void) {
  void *stuff = malloc(3432);
  tls_ptr = malloc(1234);
  atomic_store(&thread_done, true);
  while (1);
}

namespace __lsan {
  void DoLeakCheck();
}

int main(int argc, char **argv) {
  std::thread t(g);
  t.detach();

  global_ptr = malloc(1337);
  tls_ptr = malloc(420);
  memset(global_ptr, 0, 42);
  void *local_ptr = memset(calloc(16, 128), 48, 2048);
  while (!atomic_load(&thread_done));

  __lsan::DoLeakCheck();
  fprintf(stderr, "LSAN TEST COMPLETE\n");
}
