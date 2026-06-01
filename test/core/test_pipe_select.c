#include <sys/select.h>
#include <sys/ioctl.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/threading.h>
#endif

int pipe_a[2];

int get_available(int fd) {
  int avail = 0;
  assert(ioctl(fd, FIONREAD, &avail) == 0);
  printf("FIONREAD %d -> %d\n", fd, avail);
  return avail;
}

int main() {
  fd_set readfds;
  const char *t = "test\n";

  assert(pipe(pipe_a) == 0);
  assert(get_available(pipe_a[0]) == 0);
  assert(get_available(pipe_a[1]) == 0);

  FD_ZERO(&readfds);
  FD_SET(pipe_a[0], &readfds);
  write(pipe_a[1], t, strlen(t));
  assert(select(pipe_a[0] + 1, &readfds, NULL, NULL, NULL) == 1);
  assert(FD_ISSET(pipe_a[0], &readfds));

  // Slightly strange behavior here that we can use FIONREAD
  // on either the read or the write end of the pipe and get
  // the same result, but this matches linux.
  assert(get_available(pipe_a[0]) == strlen(t));
  assert(get_available(pipe_a[1]) == strlen(t));



  // Test select with timeout when no FDs are ready
  {
    int pipe_b[2];
    assert(pipe(pipe_b) == 0);

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(pipe_b[0], &fds);
    struct timeval tv = {0, 10000}; // 10ms

#ifdef __EMSCRIPTEN__
    if (emscripten_is_main_runtime_thread()) {
      printf("Main thread: expecting EINTR\n");
      int res = select(pipe_b[0] + 1, &fds, NULL, NULL, &tv);
      assert(res == -1);
      assert(errno == EINTR);
    } else {
      printf("Worker thread: expecting timeout\n");
      int res = select(pipe_b[0] + 1, &fds, NULL, NULL, &tv);
      assert(res == 0);
    }
#else
    // Native: should timeout
    printf("Native: expecting timeout\n");
    int res = select(pipe_b[0] + 1, &fds, NULL, NULL, &tv);
    assert(res == 0);
#endif

    close(pipe_b[0]);
    close(pipe_b[1]);
  }

  close(pipe_a[0]);
  close(pipe_a[1]);
  return 0;
}
