/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <sys/select.h>
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
  struct timeval tv, begin, end;

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  gettimeofday(&begin, NULL);
  assert(select(0, NULL, NULL, NULL, &tv) == 0);
  gettimeofday(&end, NULL);

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= 1000);
}

// Check if timeout works with fds without events
void test_timeout_with_fds_without_events() {
  printf("test_timeout_with_fds_without_events\n");
  struct timeval tv, begin, end;
  fd_set readfds;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(pipe_a[0], &readfds);
  gettimeofday(&begin, NULL);
  assert(select(pipe_a[0] + 1, &readfds, NULL, NULL, &tv) == 0);
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

// Check if select can unblock on an event
void test_unblock_select() {
  printf("test_unblock_select\n");
  struct timeval begin, end;
  fd_set readfds;
  pthread_t tid;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);
  assert(pipe(pipe_shared) == 0);

  FD_ZERO(&readfds);
  FD_SET(pipe_a[0], &readfds);
  FD_SET(pipe_shared[0], &readfds);
  int maxfd = (pipe_a[0] > pipe_shared[0] ? pipe_a[0] : pipe_shared[0]);
  assert(pthread_create(&tid, NULL, write_after_2s, NULL) == 0);
  gettimeofday(&begin, NULL);
  assert(select(maxfd + 1, &readfds, NULL, NULL, NULL) == 1);
  gettimeofday(&end, NULL);
  assert(FD_ISSET(pipe_shared[0], &readfds));

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert(duration >= 1000);

  pthread_join(tid, NULL);

  close(pipe_a[0]); close(pipe_a[1]);
  close(pipe_shared[0]); close(pipe_shared[1]);
}

void *do_select_in_thread(void * arg) {
  struct timeval begin, end;
  fd_set readfds;
  struct timeval tv;
  tv.tv_sec = 4;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(pipe_shared[0], &readfds);
  int maxfd = pipe_shared[0];

  gettimeofday(&begin, NULL);
  assert(select(maxfd + 1, &readfds, NULL, NULL, &tv) == 1);
  gettimeofday(&end, NULL);
  assert(FD_ISSET(pipe_shared[0], &readfds));

  int64_t duration = timeval_delta_ms(&begin, &end);
  printf(" -> duration: %lld ms\n", duration);
  assert((duration >= 1000) && (duration < 4000));

  return NULL;
}

// Check if select works in threads
void test_select_in_threads() {
  printf("test_select_in_threads\n");
  pthread_t tid1, tid2;
  const char *t = "test\n";

  assert(pipe(pipe_shared) == 0);

  assert(pthread_create(&tid1, NULL, do_select_in_thread, NULL) == 0);
  assert(pthread_create(&tid2, NULL, do_select_in_thread, NULL) == 0);

  sleep(2);
  write(pipe_shared[1], t, strlen(t));

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);

  close(pipe_shared[0]); close(pipe_shared[1]);
}

// Check if select works with ready fds
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
  int maxfd = (pipe_c[0] > pipe_d[0] ? pipe_c[0] : pipe_d[0]);

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(pipe_c[0], &readfds);
  FD_SET(pipe_d[0], &readfds);
  assert(select(maxfd + 1, &readfds, NULL, NULL, &tv) == 2);
  assert(FD_ISSET(pipe_c[0], &readfds));
  assert(FD_ISSET(pipe_d[0], &readfds));

  FD_ZERO(&readfds);
  FD_SET(pipe_c[0], &readfds);
  FD_SET(pipe_d[0], &readfds);
  assert(select(maxfd + 1, &readfds, NULL, NULL, NULL) == 2);
  assert(FD_ISSET(pipe_c[0], &readfds));
  assert(FD_ISSET(pipe_d[0], &readfds));

  close(pipe_c[0]); close(pipe_c[1]);
  close(pipe_d[0]); close(pipe_d[1]);
}

int main() {
  test_select_in_threads();
  test_timeout_without_fds();
  test_timeout_with_fds_without_events();
  test_unblock_select();
  test_ready_fds();
  printf("done\n");
  return 0;
}
