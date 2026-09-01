#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  // open of standard streams works.
  int fd_in = open("/dev/stdin", 0);
  assert(fd_in == 0);
  int fd_out = open("/dev/stdout", 0);
  assert(fd_out == 1);
  int fd_err = open("/dev/stderr", 0);
  assert(fd_err == 2);

  // ioctl and fcntl fail
  int i = ioctl(1, 0);
  assert(i < 0);
  int f = fcntl(1, 0);
  assert(f < 0);

  // write to standard streams works.
  write(1, "Hello, world!", 5);
  write(1, "\n", 1);

  // emscripten_log API works
  emscripten_console_log("log");
  // warnings/errors go to stderr
  emscripten_console_warn("warn");
  emscripten_console_error("error");
  emscripten_console_log("log2");

  // check we can call this, but the test doesn't check the output
  emscripten_get_now();

  // The epoll syscalls link against the standalone stubs and report ENOSYS.
  // A zero timeout routes through __syscall_epoll_pwait_nonblocking, a nonzero
  // one through __syscall_epoll_pwait, covering all four stubs.
  struct epoll_event ev = {};
  errno = 0;
  assert(epoll_create1(0) < 0);
  assert(errno == ENOSYS);
  errno = 0;
  assert(epoll_ctl(1, EPOLL_CTL_ADD, 0, &ev) < 0);
  assert(errno == ENOSYS);
  errno = 0;
  assert(epoll_wait(1, &ev, 1, 0) < 0);
  assert(errno == ENOSYS);
  errno = 0;
  assert(epoll_pwait(1, &ev, 1, -1, NULL) < 0);
  assert(errno == ENOSYS);

  // This doesn't do anything because we have no handler registered, but it
  // verifies that `raise` can be included in the build without any non-standard
  // imports being generated.
  raise(SIGCHLD);
  return 0;
}
