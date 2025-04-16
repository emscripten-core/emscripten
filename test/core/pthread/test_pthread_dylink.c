#include <pthread.h>
#include <stdio.h>
#include "test_pthread_dylink.h"

int foo_main = 77;

int bar_main() {
  return 42;
}

void print_addresses() {
  printf("main_module: &foo_main %p -> %d\n", &foo_main, foo_main);
  printf("main_module: &foo_side %p -> %d\n", &foo_side, foo_side);
  printf("main_module: &bar_main %p\n", &bar_main);
  printf("main_module: &bar_side %p\n", &bar_side);
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

  printf("success\n");
  return 0;
}
