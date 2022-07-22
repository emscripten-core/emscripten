#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

void longjmp_side(jmp_buf* buf);

static void* thread_main(void *arg) {
  jmp_buf buf;
  int jmpval = setjmp(buf);
  if (!jmpval) {
    printf("setjmp done\n");
    longjmp_side(&buf);
    // should never get here
    assert(false);
  } else if (jmpval == 42) {
    printf("longjmp done\n");
  } else {
    assert(false);
  }

  return NULL;
}

int main() {
  pthread_t t;
  int rc;

  rc = pthread_create(&t, NULL, thread_main, NULL);
  assert(!rc);
  rc = pthread_join(t, NULL);
  assert(!rc);

  printf("Done\n");
  return 0;
}
