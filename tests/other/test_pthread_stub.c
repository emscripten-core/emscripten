#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <pthread.h>

static void cleanup (void* arg) {
  printf("cleanup: %ld\n", (intptr_t)arg);
}

int start(void* arg) {
  assert(0);
  return 0;
}

void* start_pthread(void* arg) {
  assert(0);
  return NULL;
}

#define CHECK(X, EXPECTED) rtn = X; if (rtn != EXPECTED) printf(#X " returned %s\n", strerror(rtn)); assert(rtn == EXPECTED)
#define CHECK_FAIL(X) CHECK(X, EAGAIN)
#define CHECK_SUCCESS(X) CHECK(X, 0)

#define CHECK_C11(X, expected) rtn = X; if (rtn != expected) printf(#X " returned %d\n", rtn); assert(rtn == expected)
#define CHECK_C11_SUCCESS(X) CHECK_C11(X, thrd_success)
#define CHECK_C11_FAIL(X) CHECK_C11(X, thrd_nomem)

void test_c11_threads() {
  int rtn;
  int res;

  thrd_t thread;
  CHECK_C11_FAIL(thrd_create(&thread, start, NULL));
  // FIXME(sbc): Bug in thrd_join.c ignored pthread_join return code
  //CHECK_C11_FAIL(thrd_join(thread, &res));

  mtx_t mutex;
  CHECK_C11_SUCCESS(mtx_init(&mutex, mtx_plain));

  cnd_t cond;
  CHECK_C11_SUCCESS(cnd_init(&cond));
}

void test_pthreads() {
  int rtn;
  int res;

  // pthread_atfork should silently succeed.
  CHECK_SUCCESS(pthread_atfork(NULL, NULL, NULL));

  pthread_t thread;
  CHECK_FAIL(pthread_create(&thread, NULL, start_pthread, NULL));

  // Thread cleanup push/pop should still work for the main thrad
  pthread_cleanup_push(cleanup, (void*)42);
  pthread_cleanup_pop(1);
}

int main() {
  test_c11_threads();
  test_pthreads();
  return 0;
}
