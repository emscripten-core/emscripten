#include <assert.h>
#include <stdio.h>
#include <time.h>

long long get_time_ms() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (now.tv_sec * 1000) + (now.tv_nsec / 1000 / 1000);
}

int main(void) {
  // Sleep for 10ms relative to current time (i.e. without TIMER_ABSTIME).
  struct timespec sleep_for;
  sleep_for.tv_sec = 0;
  sleep_for.tv_nsec = 10 * 1000 * 1000;
  long long before = get_time_ms();
  printf("before: %lli\n", before);
  clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_for, NULL);
  long long after = get_time_ms();
  printf("after: %lli\n", after);

  // Check we slept at least 10ms
  assert(after > before);
  assert(after - before >= 10);
  assert(after - before < 10000);

  before = get_time_ms();
  printf("before: %lli\n", before);
  struct timespec deadline;
  clock_gettime(CLOCK_MONOTONIC, &deadline);
  deadline.tv_nsec += 20 * 1000 * 1000;
  deadline.tv_sec += (deadline.tv_nsec / (1000 * 1000 * 1000));
  deadline.tv_nsec = deadline.tv_nsec % (1000 * 1000 * 1000);
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
  after = get_time_ms();
  printf("after: %lli\n", after);

  // Check we slept at least 20ms
  assert(after > before);
  assert(after - before >= 20);
  assert(after - before < 10000);

  return 0;
}
