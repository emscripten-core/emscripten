#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int foo = 3;
// This should generate a relocation in data section
static int* _Atomic foo_addr = &foo;
// This should generate a reloction in the TLS data section
static _Thread_local int *tls_foo_addr = &foo;

void run_test() {
  printf("&foo         : %p\n", &foo);
  printf("&foo_addr    : %p\n", &foo_addr);
  printf("&tls_foo_addr: %p\n", &tls_foo_addr);
  // The values stored foo_addr and tls_foo_addr should both be
  // the address of foo
  printf("foo_add=%p tls_foo_addr=%p [delta=%ld]\n", foo_addr, tls_foo_addr, foo_addr - tls_foo_addr);
  assert(foo_addr == &foo);
  assert(tls_foo_addr == &foo);
}

void* thread_main(void* arg) {
  run_test();
  return NULL;
}

int main() {
  run_test();

  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);

  printf("done\n");
  return 0;
}
