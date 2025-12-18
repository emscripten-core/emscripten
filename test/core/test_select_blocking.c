#include <sys/select.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

// Check if timeout works without fds
void test_timeout_without_fds() {
  struct timeval tv, begin, end;

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  gettimeofday(&begin, NULL);
  assert(select(0, NULL, NULL, NULL, &tv) == 0);
  gettimeofday(&end, NULL);
  assert((end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec >= 1000000);
}

// Check if timeout works with fds without events
void test_timeout_with_fds_without_events() {
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
  assert((end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec >= 1000000);

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
  struct timeval begin, end;
  fd_set readfds;
  int maxfd;
  pthread_t tid;
  int pipe_a[2];

  assert(pipe(pipe_a) == 0);
  assert(pipe(pipe_shared) == 0);

  FD_ZERO(&readfds);
  FD_SET(pipe_a[0], &readfds);
  FD_SET(pipe_shared[0], &readfds);
  maxfd = (pipe_a[0] > pipe_shared[0] ? pipe_a[0] : pipe_shared[0]);
  assert(pthread_create(&tid, NULL, write_after_2s, NULL) == 0);
  gettimeofday(&begin, NULL);
  assert(select(maxfd + 1, &readfds, NULL, NULL, NULL) == 1);
  gettimeofday(&end, NULL);
  assert(FD_ISSET(pipe_shared[0], &readfds));
  assert((end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec >= 1000000);

  pthread_join(tid, NULL);

  close(pipe_a[0]); close(pipe_a[1]);
  close(pipe_shared[0]); close(pipe_shared[1]);
}

void *do_select_in_thread(void * arg) {
  struct timeval begin, end;
  fd_set readfds;
  int maxfd;
  struct timeval tv;
  int duration;
  tv.tv_sec = 4;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(pipe_shared[0], &readfds);
  maxfd = pipe_shared[0];

  gettimeofday(&begin, NULL);
  assert(select(maxfd + 1, &readfds, NULL, NULL, &tv) == 1);
  gettimeofday(&end, NULL);
  assert(FD_ISSET(pipe_shared[0], &readfds));
  duration = (end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec;
  assert((duration >= 1000000) && (duration < 4000000));

  return NULL;
}

// Check if select works in threads
void test_select_in_threads() {
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
  struct timeval tv;
  fd_set readfds;
  int maxfd;
  const char *t = "test\n";
  int pipe_c[2];
  int pipe_d[2];

  assert(pipe(pipe_c) == 0);
  assert(pipe(pipe_d) == 0);

  write(pipe_c[1], t, strlen(t));
  write(pipe_d[1], t, strlen(t));
  maxfd = (pipe_c[0] > pipe_d[0] ? pipe_c[0] : pipe_d[0]);

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
  return 0;
}
