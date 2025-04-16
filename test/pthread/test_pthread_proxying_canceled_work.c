#include <assert.h>
#include <emscripten/eventloop.h>
#include <emscripten/promise.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <stdio.h>

em_proxying_queue* queue;

void explode(void* arg) { assert(0 && "the work should not be run!"); }

em_promise_result_t explode_reject(void** result, void* data, void* val) {
  assert(0 && "unexpected promise result");
  return EM_PROMISE_REJECT;
}

em_promise_result_t fulfill(void** result, void* data, void* val) {
  return EM_PROMISE_FULFILL;
}

void fulfill_promise(void* arg) {
  em_promise_t promise = arg;
  emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
}

// The promises we need to wait for at the end of the test.
#define MAX_PROMISES 6
int promise_count = 0;
em_promise_t promises[MAX_PROMISES];

void add_promise(em_promise_t promise) {
  assert(promise_count < MAX_PROMISES);
  promises[promise_count++] = promise;
}

void set_flag(void* flag) {
  // Schedule the flag to be set on the next turn of the event loop so that we
  // can be sure cleanup has finished first. We need to use EM_ASM and JS here
  // because this code needs to run after the thread runtime has exited.

  // clang-format off
  EM_ASM({setTimeout(() => Atomics.store(HEAP32, $0 / 4, 1))}, flag);
  // clang-format on
}

// Used to call `set_flag` on thread exit or cancellation.
pthread_key_t dtor_key;

void* cancel_self(void* canceled) {
  pthread_setspecific(dtor_key, canceled);
  pthread_cancel(pthread_self());
  pthread_testcancel();
  assert(0 && "thread should have been canceled!");
  return NULL;
}

void* exit_self(void* exited) {
  pthread_setspecific(dtor_key, exited);
  pthread_exit(NULL);
  assert(0 && "thread should have exited!");
  return NULL;
}

void test_cancel_then_proxy() {
  printf("testing cancel followed by proxy\n");

  pthread_t thread;
  _Atomic int canceled = 0;
  pthread_create(&thread, NULL, cancel_self, &canceled);

  // Wait for the thread to be canceled.
  while (!canceled) {
  }

  // Proxying work to the thread should return an error.
  int ret = emscripten_proxy_sync(queue, thread, explode, NULL);
  assert(ret == 0);
  ret =
    emscripten_proxy_callback(queue, thread, explode, explode, explode, NULL);
  assert(ret == 0);

  // Or should result in a rejected promise.
  em_promise_t promise = emscripten_proxy_promise(queue, thread, explode, NULL);
  add_promise(emscripten_promise_then(promise, explode_reject, fulfill, NULL));
  emscripten_promise_destroy(promise);

  pthread_join(thread, NULL);
}

void test_exit_then_proxy() {
  printf("testing exit followed by proxy\n");

  pthread_t thread;
  _Atomic int exited = 0;
  pthread_create(&thread, NULL, exit_self, &exited);

  // Wait for the thread to exit.
  while (!exited) {
  }

  // Proxying work to the thread should return an error.
  int ret = emscripten_proxy_sync(queue, thread, explode, NULL);
  assert(ret == 0);
  ret =
    emscripten_proxy_callback(queue, thread, explode, explode, explode, NULL);
  assert(ret == 0);

  // Or should result in a rejected promise.
  em_promise_t promise = emscripten_proxy_promise(queue, thread, explode, NULL);
  add_promise(emscripten_promise_then(promise, explode_reject, fulfill, NULL));
  emscripten_promise_destroy(promise);

  pthread_join(thread, NULL);
}

struct flags {
  _Atomic int running;
  _Atomic int proxied;
};

void* wait_then_cancel(void* arg) {
  struct flags* flags = arg;
  flags->running = 1;

  // Wait for the proxying to start.
  while (!flags->proxied) {
  }

  pthread_cancel(pthread_self());
  pthread_testcancel();
  assert(0 && "thread should have been canceled!");
  return NULL;
}

void* wait_then_exit(void* arg) {
  struct flags* flags = arg;
  flags->running = 1;

  // Wait for the proxying to start.
  while (!flags->proxied) {
  }

  pthread_exit(NULL);
  assert(0 && "thread should have exited!");
  return NULL;
}

void test_proxy_then_cancel() {
  printf("testing proxy followed by cancel\n");

  struct flags flags = (struct flags){.running = 0, .proxied = 0};
  pthread_t thread;
  pthread_create(&thread, NULL, wait_then_cancel, &flags);

  while (!flags.running) {
  }

  // The pending proxied work should be canceled when the thread is canceled.
  em_promise_t promise = emscripten_proxy_promise(queue, thread, explode, NULL);
  add_promise(emscripten_promise_then(promise, explode_reject, fulfill, NULL));
  emscripten_promise_destroy(promise);

  promise = emscripten_promise_create();
  int ret = emscripten_proxy_callback(
    queue, thread, explode, explode, fulfill_promise, promise);
  assert(ret == 1);
  add_promise(promise);

  flags.proxied = 1;
  pthread_join(thread, NULL);
}

void test_proxy_then_exit() {
  printf("testing proxy followed by exit\n");

  struct flags flags = (struct flags){.running = 0, .proxied = 0};
  pthread_t thread;
  pthread_create(&thread, NULL, wait_then_exit, &flags);

  while (!flags.running) {
  }

  // The pending proxied work should be canceled when the thread exits.
  em_promise_t promise = emscripten_proxy_promise(queue, thread, explode, NULL);
  add_promise(emscripten_promise_then(promise, explode_reject, fulfill, NULL));
  emscripten_promise_destroy(promise);

  promise = emscripten_promise_create();
  int ret = emscripten_proxy_callback(
    queue, thread, explode, explode, fulfill_promise, promise);
  assert(ret == 1);
  add_promise(promise);

  flags.proxied = 1;
  pthread_join(thread, NULL);
}

enum proxy_kind { SYNC, CALLBACK };

struct in_progress_state {
  enum proxy_kind kind;
  int pattern_index;
  int proxier_index;
  _Atomic int running_count;
  pthread_t worker;
  pthread_t proxiers[5];
  em_proxying_ctx* ctxs[5];
  _Atomic int ctx_count;
};

struct worker_args {
  int index;
  struct in_progress_state* state;
  int* ret;
};

// The patterns of work completion to test so we sufficiently exercise the
// doubly linked list of active contexts. Numbers 1-5 indicate the order in
// which work should be completed and 0 means the work should be canceled.
int patterns[][5] = {{1, 2, 3, 4, 5},
                     {5, 4, 3, 2, 1},
                     {0, 0, 0, 0, 0},
                     {1, 0, 2, 0, 3},
                     {0, 1, 0, 2, 0},
                     {4, 2, 3, 0, 1}};

void receive_ctx(em_proxying_ctx* ctx, void* arg) {
  struct worker_args* args = arg;
  args->state->ctxs[args->index] = ctx;
  args->state->ctx_count++;
}

void* in_progress_worker(void* arg) {
  struct in_progress_state* state = arg;

  // Wait to receive all the work contexts.
  while (state->ctx_count < 5) {
    emscripten_proxy_execute_queue(queue);
  }

  // Complete the work in the order specified by the current pattern.
  for (int to_complete = 1; to_complete <= 5; to_complete++) {
    for (int i = 0; i < 5; i++) {
      if (patterns[state->pattern_index][i] == to_complete) {
        printf("finishing task %d\n", i);
        emscripten_proxy_finish(state->ctxs[i]);
      }
    }
  }
  return NULL;
}

void callback_ok(void* arg) {
  struct worker_args* args = arg;
  *args->ret = 1;
}

void callback_cancel(void* arg) {
  struct worker_args* args = arg;
  *args->ret = 0;
}

void* in_progress_proxier(void* arg) {
  struct in_progress_state* state = arg;
  int index = state->proxier_index;
  state->running_count++;

  int ret = -1;
  struct worker_args worker_args = {index, state, &ret};
  if (state->kind == SYNC) {
    // Synchronously wait for the work to be completed or canceled.
    ret = emscripten_proxy_sync_with_ctx(
      queue, state->worker, receive_ctx, &worker_args);
  } else if (state->kind == CALLBACK) {
    // Spin until we execute a callback telling us whether the work was
    // completed or canceled.
    int proxied = emscripten_proxy_callback_with_ctx(queue,
                                                     state->worker,
                                                     receive_ctx,
                                                     callback_ok,
                                                     callback_cancel,
                                                     &worker_args);
    assert(proxied == 1);
    while (ret == -1) {
      // TODO: Add a way to wait for work to be executed.
      emscripten_proxy_execute_queue(queue);
    }
  }

  // The expected result value depends on the pattern we are executing.
  assert(ret == (0 != patterns[state->pattern_index][index]));
  return NULL;
}

void test_cancel_in_progress() {
  printf("testing cancellation of in-progress work\n");

  int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
  struct in_progress_state state;

  for (state.kind = 0; state.kind <= CALLBACK; state.kind++) {
    for (state.pattern_index = 0; state.pattern_index < num_patterns;
         state.pattern_index++) {
      state.running_count = 0;
      state.ctx_count = 0;

      printf("checking pattern %d\n", state.pattern_index);

      // Spawn the worker thread.
      pthread_create(&state.worker, NULL, in_progress_worker, &state);

      // Spawn the proxier threads.
      for (state.proxier_index = 0; state.proxier_index < 5;
           state.proxier_index++) {
        pthread_create(&state.proxiers[state.proxier_index],
                       NULL,
                       in_progress_proxier,
                       &state);
        // Wait for the new proxier to start running so it gets the right index.
        while (state.running_count == state.proxier_index) {
        }
      }

      // Wait for all the threads to finish.
      for (int i = 0; i < 5; i++) {
        pthread_join(state.proxiers[i], NULL);
      }
      pthread_join(state.worker, NULL);
    }
  }
}

void do_exit(void* arg) { emscripten_runtime_keepalive_pop(); }

em_promise_result_t cleanup(void** result, void* data, void* val) {
  for (int i = 0; i < MAX_PROMISES; i++) {
    emscripten_promise_destroy(promises[i]);
  }
  em_proxying_queue_destroy(queue);
  // TODO: exit directly here
  emscripten_async_call(do_exit, NULL, 0);
  printf("done\n");
  return EM_PROMISE_FULFILL;
}

int main() {
  queue = em_proxying_queue_create();
  pthread_key_create(&dtor_key, set_flag);

  test_cancel_then_proxy();
  test_exit_then_proxy();
  test_proxy_then_cancel();
  test_proxy_then_exit();

  test_cancel_in_progress();

  // Wait for the promises to resolve.
  assert(promise_count == MAX_PROMISES);
  em_promise_t done = emscripten_promise_all(promises, NULL, MAX_PROMISES);
  emscripten_promise_destroy(
    emscripten_promise_then(done, cleanup, explode_reject, NULL));
  emscripten_promise_destroy(done);
  emscripten_runtime_keepalive_push();
}
