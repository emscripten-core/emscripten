#define _GNU_SOURCE
#include <assert.h>
#include <emscripten.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

// The worker threads we will use. `looper` sits in a loop, continuously
// processing work as it becomes available, while `returner` returns to the JS
// event loop each time it processes work.
pthread_t main_thread;
pthread_t looper;
pthread_t returner;

// The queue used to send work to both `looper` and `returner`.
em_proxying_queue* proxy_queue = NULL;

// Whether `looper` should exit.
_Atomic int should_quit = 0;

void* looper_main(void* arg) {
  while (!should_quit) {
    emscripten_proxy_execute_queue(proxy_queue);
    sched_yield();
  }
  return NULL;
}

void* returner_main(void* queue) {
  emscripten_exit_with_live_runtime();
}

typedef struct widget {
  // `val` will be stored to `out` and the current thread will be stored to
  // `thread` when the widget is run.
  int* out;
  int val;
  pthread_t thread;

  // Synchronization to allow waiting on a widget to run.
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  // Nonzero iff the widget has been run.
  int done;

  // Only used for async_as_sync tests.
  em_proxying_ctx* ctx;
} widget;

void init_widget(widget* w, int* out, int val) {
  *w = (widget){.out = out,
                .val = val,
                // .thread will be set in `run_widget`.
                .mutex = PTHREAD_MUTEX_INITIALIZER,
                .cond = PTHREAD_COND_INITIALIZER,
                .done = 0,
                .ctx = NULL};
}

void destroy_widget(widget* w) {
  pthread_mutex_destroy(&w->mutex);
  pthread_cond_destroy(&w->cond);
}

void run_widget(widget* w) {
  pthread_t self = pthread_self();
  const char* name = pthread_equal(self, main_thread) ? "main"
                     : pthread_equal(self, looper)    ? "looper"
                     : pthread_equal(self, returner)  ? "returner"
                                                      : "unknown";
  printf("running widget %d on %s\n", w->val, name);
  pthread_mutex_lock(&w->mutex);
  if (w->out) {
    *w->out = w->val;
  }
  w->thread = pthread_self();
  w->done = 1;
  pthread_mutex_unlock(&w->mutex);
  pthread_cond_broadcast(&w->cond);
}

void await_widget(widget* w) {
  pthread_mutex_lock(&w->mutex);
  while (!w->done) {
    pthread_cond_wait(&w->cond, &w->mutex);
  }
  pthread_mutex_unlock(&w->mutex);
}

// Helper functions we will proxy to perform our work.

void do_run_widget(void* arg) { run_widget((widget*)arg); }

void* do_run_widget_42(void* arg) {
  run_widget((widget*)arg);
  return (void*)42;
}

void finish_running_widget(void* arg) {
  widget* w = (widget*)arg;
  run_widget(w);
  emscripten_proxy_finish(w->ctx);
}

void start_running_widget(em_proxying_ctx* ctx, void* arg) {
  ((widget*)arg)->ctx = ctx;
  emscripten_async_call(finish_running_widget, arg, 0);
}

void start_and_finish_running_widget(em_proxying_ctx* ctx, void* arg) {
  ((widget*)arg)->ctx = ctx;
  finish_running_widget(arg);
}

// Main test functions

void test_proxy_async(void) {
  printf("Testing async proxying\n");

  int i = 0;
  widget w1, w2, w3;
  init_widget(&w1, &i, 1);
  init_widget(&w2, &i, 2);
  init_widget(&w3, &i, 3);

  // Proxy to ourselves.
  emscripten_proxy_async(proxy_queue, pthread_self(), do_run_widget, &w1);
  assert(!w1.done);
  emscripten_proxy_execute_queue(proxy_queue);
  assert(i == 1);
  assert(w1.done);
  assert(pthread_equal(w1.thread, pthread_self()));

  // Proxy to looper.
  emscripten_proxy_async(proxy_queue, looper, do_run_widget, &w2);
  await_widget(&w2);
  assert(i == 2);
  assert(w2.done);
  assert(pthread_equal(w2.thread, looper));

  // Proxy to returner.
  emscripten_proxy_async(proxy_queue, returner, do_run_widget, &w3);
  await_widget(&w3);
  assert(i == 3);
  assert(w3.done);
  assert(pthread_equal(w3.thread, returner));

  destroy_widget(&w1);
  destroy_widget(&w2);
  destroy_widget(&w3);
}

void test_proxy_sync(void) {
  printf("Testing sync proxying\n");

  int i = 0;
  widget w4, w5;
  init_widget(&w4, &i, 4);
  init_widget(&w5, &i, 5);

  // Proxy to looper.
  emscripten_proxy_sync(proxy_queue, looper, do_run_widget, &w4);
  assert(i == 4);
  assert(w4.done);
  assert(pthread_equal(w4.thread, looper));

  // Proxy to returner.
  emscripten_proxy_sync(proxy_queue, returner, do_run_widget, &w5);
  assert(i == 5);
  assert(w5.done);
  assert(pthread_equal(w5.thread, returner));

  destroy_widget(&w4);
  destroy_widget(&w5);
}

void test_proxy_sync_with_ctx(void) {
  printf("Testing sync_with_ctx proxying\n");

  int i = 0;
  widget w6, w7;
  init_widget(&w6, &i, 6);
  init_widget(&w7, &i, 7);

  // Proxy to looper.
  emscripten_proxy_sync_with_ctx(
    proxy_queue, looper, start_and_finish_running_widget, &w6);
  assert(i == 6);
  assert(w6.done);
  assert(pthread_equal(w6.thread, looper));

  // Proxy to returner.
  emscripten_proxy_sync_with_ctx(
    proxy_queue, returner, start_running_widget, &w7);
  assert(i == 7);
  assert(w7.done);
  assert(pthread_equal(w7.thread, returner));

  destroy_widget(&w6);
  destroy_widget(&w7);
}

void add_one(void* arg, void* result) {
  assert((intptr_t)result == 42);
  int* j = (int*)arg;
  *j = *j + 1;
}

thread_local int j = 0;

void test_proxy_async_with_callback(void) {
  printf("Testing async_with_callback proxying\n");

  int i = 0;
  widget w8, w9, w10;
  init_widget(&w8, &i, 8);
  init_widget(&w9, &i, 9);
  init_widget(&w10, &i, 10);

  // Proxy to ourselves.
  emscripten_proxy_async_with_callback(
    proxy_queue, pthread_self(), do_run_widget_42, &w8, add_one, &j);
  assert(!w8.done);
  assert(j == 0);
  emscripten_proxy_execute_queue(proxy_queue);
  assert(i == 8);
  assert(w8.done);
  assert(pthread_equal(w8.thread, pthread_self()));
  assert(j == 1);

  // Proxy to looper.
  emscripten_proxy_async_with_callback(
    proxy_queue, looper, do_run_widget_42, &w9, add_one, &j);
  await_widget(&w9);
  assert(i == 9);
  assert(w9.done);
  assert(pthread_equal(w9.thread, looper));
  assert(j == 1);
  // TODO: Add a way to wait for work before executing it.
  while (j < 2) {
    emscripten_proxy_execute_queue(proxy_queue);
  }
  assert(j == 2);

  // Proxy to returner.
  emscripten_proxy_async_with_callback(
    proxy_queue, returner, do_run_widget_42, &w10, add_one, &j);
  await_widget(&w10);
  assert(i == 10);
  assert(w10.done);
  assert(pthread_equal(w10.thread, returner));
  assert(j == 2);
  emscripten_proxy_execute_queue(proxy_queue);
  // TODO: Add a way to wait for work before executing it.
  while (j < 3) {
    emscripten_proxy_execute_queue(proxy_queue);
  }
  assert(j == 3);

  destroy_widget(&w8);
  destroy_widget(&w9);
  destroy_widget(&w10);
}

typedef struct increment_to_arg {
  em_proxying_queue* queue;
  int* ip;
  int i;
} increment_to_arg;

void increment_to(void* arg_p) {
  increment_to_arg* arg = (increment_to_arg*)arg_p;

  // Try executing the queue; since the queue is already being executed, this
  // shouldn't do anything and *arg->ip should still be one less than arg->i
  // afterward.
  emscripten_proxy_execute_queue(arg->queue);

  assert(*arg->ip == arg->i - 1);
  *arg->ip = arg->i;
  free(arg);
}

void test_tasks_queue_growth(void) {
  printf("Testing tasks queue growth\n");

  em_proxying_queue* queue = em_proxying_queue_create();
  assert(proxy_queue != NULL);

  int incremented = 0;

  // Initial queue capacity is 128. Force that to double twice with the head at
  // index 0 by inserting more than 256 items.
  for (int i = 1; i <= 300; i++) {
    increment_to_arg* arg = malloc(sizeof(increment_to_arg));
    *arg = (increment_to_arg){queue, &incremented, i};
    int res = emscripten_proxy_async(queue, pthread_self(), increment_to, arg);
    assert(res == 1);
  }

  // Drain the queue, moving the head somewhere into the middle of the buffer of
  // capacity 512.
  emscripten_proxy_execute_queue(queue);
  assert(incremented == 300);

  // Double the queue size twice more by inserting more than 1024 items.
  for (int i = 301; i <= 1500; i++) {
    increment_to_arg* arg = malloc(sizeof(increment_to_arg));
    *arg = (increment_to_arg){queue, &incremented, i};
    int res = emscripten_proxy_async(queue, pthread_self(), increment_to, arg);
    assert(res == 1);
  }

  // Drain the queue again.
  emscripten_proxy_execute_queue(queue);
  assert(incremented == 1500);

  em_proxying_queue_destroy(queue);
}

typedef struct proxying_queue_growth_arg {
  em_proxying_queue* queue;
  pthread_t a;
  pthread_t b;
  _Atomic int work_count;
} proxying_queue_growth_arg;

void trivial_work(void* arg) {
  printf("work\n");
  (*(_Atomic int*)arg)++;
}

void grow_proxying_queue(void* arg_p) {
  // Add task_queues for two new threads, causing a reallocation of the
  // `em_proxying_queue`'s task_queues array the first time this is called.
  proxying_queue_growth_arg* arg = (proxying_queue_growth_arg*)arg_p;
  emscripten_proxy_async(arg->queue, arg->a, trivial_work, &arg->work_count);
  emscripten_proxy_async(arg->queue, arg->b, trivial_work, &arg->work_count);
}

void test_proxying_queue_growth(void) {
  printf("Testing proxying queue growth\n");

  proxying_queue_growth_arg arg;
  arg.queue = em_proxying_queue_create();
  assert(arg.queue != NULL);

  pthread_create(&arg.a, NULL, returner_main, arg.queue);
  pthread_create(&arg.b, NULL, returner_main, arg.queue);

  arg.work_count = 0;

  // Queue a task for the current thread, allocating an array of one task_queue.
  // Then when the task is executed, work is queued on two new threads, bumping
  // up the array size to 4 and causing it to be reallocated elsewhere. Make
  // sure we correctly handle this reallocation in the middle of executing the
  // queue.
  emscripten_proxy_async(arg.queue, pthread_self(), grow_proxying_queue, &arg);
  emscripten_proxy_execute_queue(arg.queue);

  while (arg.work_count < 2) {
    sched_yield();
  }

  // Do it again to make sure the queue was left in a valid state. Specifically,
  // if the reallocation is not handled correctly, the recursion guard might not
  // have been updated correctly, so the work will not be completed.
  emscripten_proxy_async(arg.queue, pthread_self(), grow_proxying_queue, &arg);
  emscripten_proxy_execute_queue(arg.queue);

  while (arg.work_count < 4) {
    sched_yield();
  }

  // Clean up.
  pthread_cancel(arg.a);
  pthread_cancel(arg.b);
  pthread_join(arg.a, NULL);
  pthread_join(arg.b, NULL);
  em_proxying_queue_destroy(arg.queue);
}

int main(int argc, char* argv[]) {
  main_thread = pthread_self();

  proxy_queue = em_proxying_queue_create();
  assert(proxy_queue != NULL);

  pthread_create(&looper, NULL, looper_main, NULL);
  pthread_create(&returner, NULL, returner_main, proxy_queue);

  test_proxy_async();
  test_proxy_sync();
  test_proxy_sync_with_ctx();
  test_proxy_async_with_callback();

  should_quit = 1;
  pthread_join(looper, NULL);

  pthread_cancel(returner);
  pthread_join(returner, NULL);

  em_proxying_queue_destroy(proxy_queue);

  test_tasks_queue_growth();
  test_proxying_queue_growth();

  printf("done\n");
}
