#include <sys/select.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int pipe_a[2];

int main() {
  fd_set readfds;
  const char *t = "test\n";

  assert(pipe(pipe_a) == 0);
  FD_ZERO(&readfds);
  FD_SET(pipe_a[0], &readfds);
  write(pipe_a[1], t, strlen(t));
  assert(select(pipe_a[0] + 1, &readfds, NULL, NULL, NULL) == 1);
  assert(FD_ISSET(pipe_a[0], &readfds));

  close(pipe_a[0]); close(pipe_a[1]);

  return 0;
}
