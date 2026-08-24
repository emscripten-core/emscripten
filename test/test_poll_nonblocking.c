#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

// A zero-timeout poll() is a non-suspending probe, so it must work from any
// context. The property under test with -sJSPI: it stays callable from an
// event-loop callback, where a suspending call would trap (Suspending
// imports require a stack entered via a promising export).

static void probe(const char* where) {
  struct pollfd pfd;
  pfd.fd = STDOUT_FILENO;
  pfd.events = POLLOUT;
  pfd.revents = 0;
  int n = poll(&pfd, 1, 0);
  assert(n == 1);
  assert(pfd.revents & POLLOUT);
  printf("poll probe ok from %s\n", where);
}

static void on_timeout(void* user_data) {
  probe("callback");
  // A true shutdown even where the runtime is kept alive (e.g. worker
  // threads under PROXY_TO_PTHREAD).
  emscripten_force_exit(0);
}

int main(void) {
  probe("main");
  emscripten_set_timeout(on_timeout, 0, NULL);
  emscripten_exit_with_live_runtime();
  return 99;
}
