#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <emscripten/eventloop.h>

bool got_timeout = false;
bool got_microtask = false;

void callback_timeout(void* user_data) {
  assert(got_microtask);
  printf("callback_timeout: %ld\n", (intptr_t)user_data);
  got_timeout = true;
}

void callback_microtask(void* user_data) {
  assert(!got_timeout);
  printf("callback_microtask: %ld\n", (intptr_t)user_data);
  got_microtask = true;
}

int main() {
  emscripten_set_timeout(callback_timeout, 0, (void*)42);
  emscripten_queue_microtask(callback_microtask, (void*)43);
  printf("done main\n");
  return 0;
}
