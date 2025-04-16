#include <assert.h>
#include <stdio.h>
#include <pthread.h>

int get_side_tls();
int get_side_tls2();
int* get_side_tls_address();
int* get_side_tls_address2();

static __thread int main_tls = 10;
static __thread int main_tls2 = 11;
extern __thread int side_tls;
extern __thread int side_tls2;

int get_main_tls() {
  return main_tls;
}

int get_main_tls2() {
  return main_tls2;
}

int* get_main_tls_address() {
  return &main_tls;
}

void report_tls() {
  //printf("side_tls address: %p\n", get_side_tls_address());
  printf("side_tls  value  : %d %d\n", get_side_tls(), get_side_tls2());
  printf("side_tls direct  : %d %d\n", side_tls, side_tls2);
  //printf("main_tls address: %p\n", get_main_tls_address());
  printf("main_tls  value  : %d %d\n", get_main_tls(), get_main_tls2());
  printf("main_tls direct  : %d %d\n", main_tls, main_tls2);
  assert(get_side_tls() == side_tls);
  assert(get_side_tls2() == side_tls2);
}

void test_tls(int inc) {
  report_tls();
  int* m = get_main_tls_address();
  int* s = get_side_tls_address();
  *m = *m + inc;
  *s = *s + inc;
  printf("increment done\n");
  report_tls();
  printf("\n");
}

void* thread_main(void* arg) {
  printf("thread: 2\n");
  test_tls(10);
  return NULL;
}

int main() {
  printf("thread: 1\n");
  test_tls(20);

  int save_side = get_side_tls();
  int save_main = get_main_tls();

  // Now do the same on a second thread
  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);

  // Check that the second thread does not effect the values
  // of on the main thread.
  assert(save_side == get_side_tls());
  assert(save_main == get_main_tls());

  printf("success\n");
}
