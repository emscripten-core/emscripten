/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// poll() suspended on the main thread (via ASYNCIFY/JSPI) must be woken when
// another thread writes to the polled pipe. Unlike test_poll_blocking.c this
// must not be built with PROXY_TO_PTHREAD: the poll has to run on the real main
// thread, where it cannot block and suspends instead.

#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define WRITE_DELAY_MS 300
// Long enough that waking only at the timeout is clearly distinguishable from
// being woken by the write.
#define POLL_TIMEOUT_MS 3000
// It is possible for the node timers (such as setTimeout or Atomics.wait) to
// wake up slightly earlier than requested. Because we measure times accurately
// using clock_gettime, we give tests a 20 milliseconds error margin to avoid
// flaky timeouts.
#define TIMEOUT_MARGIN_MS 20

int64_t timespec_delta_ms(struct timespec* begin, struct timespec* end) {
  int64_t delta_sec = end->tv_sec - begin->tv_sec;
  int64_t delta_nsec = end->tv_nsec - begin->tv_nsec;

  assert(delta_sec >= 0);
  assert(delta_nsec > -1000000000 && delta_nsec < 1000000000);

  int64_t delta_ms = (delta_sec * 1000000000LL + delta_nsec) / 1000000;
  assert(delta_ms >= 0);
  return delta_ms;
}

int pipe_fds[2];

void* write_after_sleep(void* arg) {
  usleep(WRITE_DELAY_MS * 1000);
  printf("write_after_sleep\n");
  const char* t = "test\n";
  assert(write(pipe_fds[1], t, strlen(t)) == (ssize_t)strlen(t));
  return NULL;
}

int main() {
  struct timespec begin = {0};
  struct timespec end = {0};
  pthread_t tid;

  assert(pipe(pipe_fds) == 0);
  struct pollfd fds = {pipe_fds[0], POLLIN, 0};

  clock_gettime(CLOCK_MONOTONIC, &begin);
  assert(pthread_create(&tid, NULL, write_after_sleep, NULL) == 0);
  // A finite timeout, so a missed wakeup fails the duration check below
  // instead of hanging the test.
  assert(poll(&fds, 1, POLL_TIMEOUT_MS) == 1);
  clock_gettime(CLOCK_MONOTONIC, &end);
  assert(fds.revents & POLLIN);

  int64_t duration = timespec_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= WRITE_DELAY_MS - TIMEOUT_MARGIN_MS);
  // Woken by the write, not by the timeout.
  assert(duration < POLL_TIMEOUT_MS / 2);

  // The writer is done by now, so joining from the main thread is brief.
  pthread_join(tid, NULL);
  close(pipe_fds[0]);
  close(pipe_fds[1]);
  printf("done\n");
  return 0;
}
