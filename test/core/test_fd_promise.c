/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_fd_promise(): a promise of an fd's readiness. A pipe's read end
 * is pending until written to, then fulfilled with POLLIN; an already-ready
 * fd and a bad fd (POLLNVAL) fulfil at once. Each then-callback runs from the
 * event loop, so main() returns with the promise chain in flight and the
 * runtime is held alive by it until the last callback.
 */

#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/promise.h>
#include <poll.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int p[2];
int stage;

em_promise_result_t fail(void** result, void* data, void* value) {
  assert(0 && "promise rejected");
}

em_promise_result_t on_nval(void** result, void* data, void* value) {
  assert(stage++ == 2);
  assert((intptr_t)value == POLLNVAL);
  printf("done\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  // A promise callback does not exit the thread itself when the last keepalive
  // drops, so the worker would otherwise idle forever.
  exit(0);
#endif
  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_writable(void** result, void* data, void* value) {
  assert(stage++ == 1);
  assert((intptr_t)value == POLLOUT);
  assert(close(p[0]) == 0 && close(p[1]) == 0);
  // A closed fd reports POLLNVAL at once.
  em_promise_t nval = emscripten_fd_promise(p[0], POLLIN);
  em_promise_t done = emscripten_promise_then(nval, on_nval, fail, NULL);
  emscripten_promise_destroy(nval);
  emscripten_promise_destroy(done);
  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_readable(void** result, void* data, void* value) {
  assert(stage++ == 0);
  assert((intptr_t)value == POLLIN);
  char c;
  assert(read(p[0], &c, 1) == 1 && c == 'x');
  // An already-writable fd fulfils without waiting.
  em_promise_t writable = emscripten_fd_promise(p[1], POLLOUT);
  em_promise_t next = emscripten_promise_then(writable, on_writable, fail, NULL);
  emscripten_promise_destroy(writable);
  emscripten_promise_destroy(next);
  return EM_PROMISE_FULFILL;
}

void write_later(void* arg) {
  assert(stage == 0);
  assert(write(p[1], "x", 1) == 1);
}

int main() {
  assert(pipe(p) == 0);
  em_promise_t readable = emscripten_fd_promise(p[0], POLLIN);
  em_promise_t next = emscripten_promise_then(readable, on_readable, fail, NULL);
  emscripten_promise_destroy(readable);
  emscripten_promise_destroy(next);
  // Nothing is readable yet: the write that fulfils it comes from the loop.
  emscripten_set_timeout(write_later, 10, NULL);
  return 0;
}
