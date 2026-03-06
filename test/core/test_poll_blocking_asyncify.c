/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Duplicate of test_poll_blocking.c but without the pthread stuff.  Instead
// we use `emscripten_set_timeout` and depend on ASYNCIFY to suspend the Wasm.

#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <emscripten/eventloop.h>

int64_t timeval_delta_ms(struct timeval* begin, struct timeval* end) {
  int64_t delta_s = end->tv_sec - begin->tv_sec;
  int64_t delta_us =  end->tv_usec -  begin->tv_usec;
  assert(delta_s >= 0);
  return (delta_s * 1000) + (delta_us / 1000);
}

// Check if timeout works without fds
void test_timeout_without_fds() {
  printf("test_timeout_without_fds\n");
  struct timeval begin, end;

  gettimeofday(&begin, NULL);
  assert(poll(NULL, 0, 1000) == 0);
  gettimeofday(&end, NULL);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= 1000);
}

int pipe_shared[2];

void write_to_pipe(void * arg) {
  printf("write_to_pipe\n");
  const char *t = "test\n";
  write(pipe_shared[1], t, strlen(t));
}

// Check if poll can unblock on an event
void test_unblock_poll() {
  printf("test_unblock_poll\n");
  struct timeval begin, end;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);
  assert(pipe(pipe_shared) == 0);

  struct pollfd fds[2] = {
    {pipe_a[0], POLLIN, 0},
    {pipe_shared[0], POLLIN, 0},
  };
  emscripten_set_timeout(write_to_pipe, 1000, NULL);
  gettimeofday(&begin, NULL);
  assert(poll(fds, 2, -1) == 1);
  gettimeofday(&end, NULL);
  assert(fds[1].revents & POLLIN);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= 1000);

  close(pipe_a[0]); close(pipe_a[1]);
  close(pipe_shared[0]); close(pipe_shared[1]);
}

int main() {
  test_timeout_without_fds();
  test_unblock_poll();
  printf("done\n");
  return 0;
}
