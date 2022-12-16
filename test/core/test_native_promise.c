#include <stdio.h>
#include <emscripten/eventloop.h>
#include <emscripten/em_js.h>

void timer_expired(void* user_data) {
  int promise_id = (intptr_t)user_data;
  printf("timer_expired promise_id=%d\n", promise_id);
  emscripten_promise_resolve(promise_id, (void*)100);
}

void async_work(void* user_data, int promise_id) {
  printf("async_work started: user_data=%ld promise_id=%d\n",
         (intptr_t)user_data,
         promise_id);
  emscripten_set_timeout(timer_expired, 1000, (void*)(intptr_t)promise_id);
}

EM_JS_DEPS(deps, "$getPromise,$runtimeKeepalivePush,$runtimeKeepalivePop");

EM_JS(void, exit_after_promises, (int id1, int id2), {
  err(`exit_after_promises: ${id1} ${id2}`);
  runtimeKeepalivePush();
  getPromise(id1).then((value) => {
    getPromise(id2).then((value2) => {
      err(`promises resolved (${value}, ${value2}); exiting`);
      runtimeKeepalivePop();
    });
  });
});

int main() {
  int id1 = emscripten_promise_create(async_work, (void*)42);
  int id2 = emscripten_promise_create(async_work, (void*)99);
  exit_after_promises(id1, id2);
  printf("main done\n");
  return 0;
}
