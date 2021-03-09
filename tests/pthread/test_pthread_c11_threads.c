#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <threads.h>

once_flag flag = ONCE_FLAG_INIT;
_Atomic int counter = 0;

void do_once(void) {
  counter++;
  printf("in do_once\n");
}

int thread_main(void* arg) {
  printf("in thread_main %p\n", thrd_current());
  call_once(&flag, do_once);
  printf("done thread_main\n");
  return 42;
}

int run_with_exit(void* arg) {
  thrd_yield();
  thrd_exit(43);
}

#ifdef REPORT_RESULT
#define EM_ASSERT(x) if (!(x)) { REPORT_RESULT(1); abort(); }
#else
#define EM_ASSERT(x) assert(x)
#endif

int main(int argc, char* argv[]) {
  int result = 0;
  printf("thrd_current: %p\n", thrd_current());

  // Test call_once
  thrd_t t1;
  thrd_t t2;
  thrd_t t3;
  thrd_t t4;

  EM_ASSERT(thrd_create(&t1, thread_main, NULL) == thrd_success);
  EM_ASSERT(thrd_create(&t2, thread_main, NULL) == thrd_success);
  EM_ASSERT(thrd_create(&t3, thread_main, NULL) == thrd_success);
  EM_ASSERT(thrd_create(&t4, thread_main, NULL) == thrd_success);

  EM_ASSERT(!thrd_equal(t1, t2));
  EM_ASSERT(thrd_equal(t1, t1));

  EM_ASSERT(thrd_join(t1, &result) == thrd_success);
  EM_ASSERT(thrd_join(t2, &result) == thrd_success);
  EM_ASSERT(thrd_join(t3, &result) == thrd_success);
  EM_ASSERT(thrd_join(t4, &result) == thrd_success);
  EM_ASSERT(result == 42);
  EM_ASSERT(counter == 1);

  // Test thrd_exit return value
  thrd_t t5;
  EM_ASSERT(thrd_create(&t5, run_with_exit, NULL) == thrd_success);
  EM_ASSERT(thrd_join(t5, &result) == thrd_success);
  EM_ASSERT(result == 43);

  // Test thrd_detach
  thrd_t t6;
  EM_ASSERT(thrd_create(&t6, thread_main, NULL) == thrd_success);
  EM_ASSERT(thrd_detach(t6) == thrd_success);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  printf("done!\n");
  return 0;
}
