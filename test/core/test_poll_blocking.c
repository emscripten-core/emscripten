/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Duplicate of test_select_blocking.c using poll() instead of select()

#include <poll.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

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

// Check if timeout works with fds without events
void test_timeout_with_fds_without_events() {
  printf("test_timeout_with_fds_without_events\n");
  struct timeval begin, end;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);

  gettimeofday(&begin, NULL);
  struct pollfd fds = {pipe_a[0], 0, 0};
  assert(poll(&fds, 1, 1000) == 0);
  gettimeofday(&end, NULL);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= 1000);

  close(pipe_a[0]); close(pipe_a[1]);
}

int pipe_shared[2];

void *write_after_2s(void * arg) {
  const char *t = "test\n";

  sleep(2);
  write(pipe_shared[1], t, strlen(t));

  return NULL;
}

// Check if poll can unblock on an event
void test_unblock_poll() {
  printf("test_unblock_poll\n");
  struct timeval begin, end;
  pthread_t tid;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);
  assert(pipe(pipe_shared) == 0);

  struct pollfd fds[2] = {
    {pipe_a[0], POLLIN, 0},
    {pipe_shared[0], POLLIN, 0},
  };
  assert(pthread_create(&tid, NULL, write_after_2s, NULL) == 0);
  gettimeofday(&begin, NULL);
  assert(poll(fds, 2, -1) == 1);
  gettimeofday(&end, NULL);
  assert(fds[1].revents & POLLIN);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= 1000);

  pthread_join(tid, NULL);

  close(pipe_a[0]); close(pipe_a[1]);
  close(pipe_shared[0]); close(pipe_shared[1]);
}

void *do_poll_in_thread(void * arg) {
  struct timeval begin, end;

  gettimeofday(&begin, NULL);
  struct pollfd fds = {pipe_shared[0], POLLIN, 0};
  assert(poll(&fds, 1, 4000) == 1);
  gettimeofday(&end, NULL);
  assert(fds.events & POLLIN);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert((duration >= 1000) && (duration < 4000));

  return NULL;
}

// Check if poll works in threads
void test_poll_in_threads() {
  printf("test_poll_in_threads\n");
  pthread_t tid1, tid2;
  const char *t = "test\n";

  assert(pipe(pipe_shared) == 0);

  assert(pthread_create(&tid1, NULL, do_poll_in_thread, NULL) == 0);
  assert(pthread_create(&tid2, NULL, do_poll_in_thread, NULL) == 0);

  sleep(2);
  write(pipe_shared[1], t, strlen(t));

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);

  close(pipe_shared[0]); close(pipe_shared[1]);
}

// Check if poll works with ready fds
void test_ready_fds() {
  printf("test_ready_fds\n");
  struct timeval tv;
  fd_set readfds;
  const char *t = "test\n";
  int pipe_c[2];
  int pipe_d[2];

  assert(pipe(pipe_c) == 0);
  assert(pipe(pipe_d) == 0);

  write(pipe_c[1], t, strlen(t));
  write(pipe_d[1], t, strlen(t));

  struct pollfd fds[2] = {
    {pipe_c[0], POLLIN, 0},
    {pipe_d[0], POLLIN, 0},
  };

  assert(poll(fds, 2, 0) == 2);
  assert(fds[0].revents & POLLIN);
  assert(fds[1].revents & POLLIN);

  fds[0].revents = 0;
  fds[1].revents = 0;

  assert(poll(fds, 2, 0) == 2);
  assert(fds[0].revents & POLLIN);
  assert(fds[1].revents & POLLIN);

  close(pipe_c[0]); close(pipe_c[1]);
  close(pipe_d[0]); close(pipe_d[1]);
}

int main() {
  test_poll_in_threads();
  test_timeout_without_fds();
  test_timeout_with_fds_without_events();
  test_unblock_poll();
  test_ready_fds();
  printf("done\n");
  return 0;
}
