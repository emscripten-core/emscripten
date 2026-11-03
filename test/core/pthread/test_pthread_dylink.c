#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <emscripten/em_asm.h>
#include "test_pthread_dylink.h"

int foo_main = 77;

int bar_main() {
  return 42;
}

// Not exported, so the dynamic linker never looks it up.
static int baz_main() {
  return 43;
}

void print_addresses() {
  printf("main_module: &foo_main %p -> %d\n", &foo_main, foo_main);
  printf("main_module: &foo_side %p -> %d\n", &foo_side, foo_side);
  printf("main_module: &bar_main %p\n", &bar_main);
  printf("main_module: &bar_side %p\n", &bar_side);
  // Each function has one address in every module and every thread.
  assert(bar_main_addr_side() == &bar_main);
  // addFunction on a function that is already in the table returns its
  // existing address, including in a worker for a main-module function that is
  // not exported.
  bar_func baz = &baz_main;
  void* addr = EM_ASM_PTR({ return addFunction(getWasmTableEntry($0)); }, baz);
  assert(addr == (void*)baz);
}

void* thread_main(void* arg) {
  printf("secondary thread:\n");
  print_addresses();
  print_addresses_side();
  printf("\n");
  return NULL;
}

int main(int argc, char* argv[]) {
  printf("main thread:\n");
  print_addresses();
  print_addresses_side();
  printf("\n");

  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, NULL);

  printf("done\n");
  return 0;
}
