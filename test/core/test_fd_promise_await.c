/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/promise.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int p[2];

void write_later(void* arg) {
  assert(write(p[1], "x", 1) == 1);
}

int main() {
  assert(pipe(p) == 0);
  emscripten_set_timeout(write_later, 10, NULL);
  em_promise_t readable = emscripten_fd_promise(p[0], POLLIN);
  em_settled_result_t r = emscripten_promise_await(readable);
  emscripten_promise_destroy(readable);
  assert(r.result == EM_PROMISE_FULFILL);
  assert((intptr_t)r.value == POLLIN);
  char c;
  assert(read(p[0], &c, 1) == 1 && c == 'x');
  printf("done\n");
  return 0;
}
