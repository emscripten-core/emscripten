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

#define TIMEOUT_MS 300

void sleep_ms(int ms) {
  usleep(ms * 1000);
}

int64_t timeval_delta_ms(struct timespec* begin, struct timespec* end) {
  struct timespec diff = {
    .tv_sec = end->tv_sec - begin->tv_sec,
    .tv_nsec = end->tv_nsec - begin->tv_nsec
  };

  if (diff.tv_nsec < 0) {
    diff.tv_nsec += 1000000000;
    diff.tv_sec -= 1;
  }

  assert(diff.tv_sec >= 0);

  return (diff.tv_sec * 1000) + (diff.tv_nsec / 1000000);
}

// Check if timeout works without fds
void test_timeout_without_fds() {
  printf("test_timeout_without_fds\n");
  struct timespec begin, end;

  clock_gettime(CLOCK_MONOTONIC, &begin);
  assert(poll(NULL, 0, TIMEOUT_MS) == 0);
  clock_gettime(CLOCK_MONOTONIC, &end);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= TIMEOUT_MS);
}

// Check if timeout works with fds without events
void test_timeout_with_fds_without_events() {
  printf("test_timeout_with_fds_without_events\n");
  struct timespec begin, end;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);

  clock_gettime(CLOCK_MONOTONIC, &begin);
  struct pollfd fds = {pipe_a[0], 0, 0};
  assert(poll(&fds, 1, TIMEOUT_MS) == 0);
  clock_gettime(CLOCK_MONOTONIC, &end);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= TIMEOUT_MS);

  close(pipe_a[0]); close(pipe_a[1]);
}

int pipe_shared[2];

void *write_after_sleep(void * arg) {
  const char *t = "test\n";

  sleep_ms(TIMEOUT_MS);
  write(pipe_shared[1], t, strlen(t));

  return NULL;
}

// Check if poll can unblock on an event
void test_unblock_poll() {
  printf("test_unblock_poll\n");
  struct timespec begin, end;
  pthread_t tid;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);
  assert(pipe(pipe_shared) == 0);

  struct pollfd fds[2] = {
    {pipe_a[0], POLLIN, 0},
    {pipe_shared[0], POLLIN, 0},
  };
  clock_gettime(CLOCK_MONOTONIC, &begin);
  assert(pthread_create(&tid, NULL, write_after_sleep, NULL) == 0);
  assert(poll(fds, 2, -1) == 1);
  clock_gettime(CLOCK_MONOTONIC, &end);
  assert(fds[1].revents & POLLIN);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= TIMEOUT_MS);

  pthread_join(tid, NULL);

  close(pipe_a[0]); close(pipe_a[1]);
  close(pipe_shared[0]); close(pipe_shared[1]);
}

void *do_poll_in_thread(void * arg) {
  struct timespec begin, end;

  clock_gettime(CLOCK_MONOTONIC, &begin);
  struct pollfd fds = {pipe_shared[0], POLLIN, 0};
  assert(poll(&fds, 1, 4000) == 1);
  clock_gettime(CLOCK_MONOTONIC, &end);
  assert(fds.revents & POLLIN);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert((duration >= TIMEOUT_MS) && (duration < 4000));

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

  sleep_ms(2 * TIMEOUT_MS);
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
