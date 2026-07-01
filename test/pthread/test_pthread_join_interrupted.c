#define _GNU_SOURCE // for pthread_timedjoin_np
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

_Atomic bool got_alarm = false;

void* thread_main(void* arg) {
  printf("thread_main\n");
  while (!got_alarm) {
    // Sleep in 1ms chunks until the main thread recieves the alarm.
    usleep(1 * 1000);
  }
  usleep(100 * 1000);
  return (void*)42;
}

void my_handler(int signum) {
  assert(signum == SIGALRM);
  got_alarm = true;
}

int main() {
  int rtn;
  printf("main\n");
  pthread_t t;
  // Set an alarm to fire while we are waiting for the thread to exit.
  signal(SIGALRM, my_handler);
  alarm(1);
  rtn = pthread_create(&t, NULL, thread_main, NULL);
  assert(rtn == 0);
  void* result;
  // Test that the alarm does not interrupt the join operation.  i.e.
  // join should not return fail or return early here.
  rtn = pthread_join(t, &result);
  assert(rtn == 0);
  assert((int)result == 42);

  // Same again but this time with a pthread_timedjoin_np
  got_alarm = false;
  result = 0;
  alarm(1);
  rtn = pthread_create(&t, NULL, thread_main, NULL);
  assert(rtn == 0);
  struct timespec timeout;
  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += 3;
  rtn = pthread_timedjoin_np(t, &result, &timeout);
  assert(rtn == 0);
  assert((int)result == 42);

  printf("done\n");
  return 0;
}
