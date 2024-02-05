#include <stdio.h>
#include <threads.h>

static thread_local int baz = 42;
static thread_local int wiz = 43;

int get_baz() {
  return baz;
}

int get_wiz() {
  return wiz;
}

void sidey() {
  printf("side TLS: %d %d\n", get_baz(), get_wiz());
}
