/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "em_task_queue.h"
#include "thread_mailbox.h"
#include "threading_internal.h"

struct em_proxying_queue {
  // Protects all accesses to em_task_queues, size, and capacity.
  pthread_mutex_t mutex;
  // `size` task queue pointers stored in an array of size `capacity`.
  em_task_queue** task_queues;
  int size;
  int capacity;
};

// The system proxying queue.
static em_proxying_queue system_proxying_queue = {
  .mutex = PTHREAD_MUTEX_INITIALIZER,
  .task_queues = NULL,
  .size = 0,
  .capacity = 0,
};

em_proxying_queue* emscripten_proxy_get_system_queue(void) {
  return &system_proxying_queue;
}

em_proxying_queue* em_proxying_queue_create(void) {
  // Allocate the new queue.
  em_proxying_queue* q = malloc(sizeof(em_proxying_queue));
  if (q == NULL) {
    return NULL;
  }
  *q = (em_proxying_queue){
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .task_queues = NULL,
    .size = 0,
    .capacity = 0,
  };
  return q;
}

void em_proxying_queue_destroy(em_proxying_queue* q) {
  assert(q != NULL);
  assert(q != &system_proxying_queue && "cannot destroy system proxying queue");

  pthread_mutex_destroy(&q->mutex);
  for (int i = 0; i < q->size; i++) {
    em_task_queue_destroy(q->task_queues[i]);
  }
  free(q->task_queues);
  free(q);
}

// Not thread safe. Returns NULL if there are no tasks for the thread.
static em_task_queue* get_tasks_for_thread(em_proxying_queue* q,
                                           pthread_t thread) {
  assert(q != NULL);
  for (int i = 0; i < q->size; i++) {
    if (pthread_equal(q->task_queues[i]->thread, thread)) {
      return q->task_queues[i];
    }
  }
  return NULL;
}

// Not thread safe.
static em_task_queue* get_or_add_tasks_for_thread(em_proxying_queue* q,
                                                  pthread_t thread) {
  em_task_queue* tasks = get_tasks_for_thread(q, thread);
  if (tasks != NULL) {
    return tasks;
  }
  // There were no tasks for the thread; initialize a new em_task_queue. If
  // there are not enough queues, allocate more.
  if (q->size == q->capacity) {
    int new_capacity = q->capacity == 0 ? 1 : q->capacity * 2;
    em_task_queue** new_task_queues =
      realloc(q->task_queues, sizeof(em_task_queue*) * new_capacity);
    if (new_task_queues == NULL) {
      return NULL;
    }
    q->task_queues = new_task_queues;
    q->capacity = new_capacity;
  }
  // Initialize the next available task queue.
  tasks = em_task_queue_create(thread);
  if (tasks == NULL) {
    return NULL;
  }
  q->task_queues[q->size++] = tasks;
  return tasks;
}

void emscripten_proxy_execute_queue(em_proxying_queue* q) {
  assert(q != NULL);
  assert(pthread_self());

  // Recursion guard to avoid infinite recursion when we arrive here from the
  // pthread_lock call below that executes the system queue. The per-task_queue
  // recursion lock can't catch these recursions because it can only be checked
  // after the lock has been acquired.
  static _Thread_local int executing_system_queue = 0;
  int is_system_queue = q == &system_proxying_queue;
  if (is_system_queue) {
    if (executing_system_queue) {
      return;
    }
    executing_system_queue = 1;
  }

  pthread_mutex_lock(&q->mutex);
  em_task_queue* tasks = get_tasks_for_thread(q, pthread_self());
  pthread_mutex_unlock(&q->mutex);

  if (tasks != NULL && !tasks->processing) {
    // Found the task queue and it is not already being processed; process it.
    em_task_queue_execute(tasks);
  }

  if (is_system_queue) {
    executing_system_queue = 0;
  }
}

static int do_proxy(em_proxying_queue* q, pthread_t target_thread, task t) {
  assert(q != NULL);
  pthread_mutex_lock(&q->mutex);
  em_task_queue* tasks = get_or_add_tasks_for_thread(q, target_thread);
  pthread_mutex_unlock(&q->mutex);
  if (tasks == NULL) {
    return 0;
  }

  return em_task_queue_send(tasks, t);
}

int emscripten_proxy_async(em_proxying_queue* q,
                           pthread_t target_thread,
                           void (*func)(void*),
                           void* arg) {
  return do_proxy(q, target_thread, (task){func, NULL, arg});
}

enum ctx_kind { SYNC, CALLBACK };

enum ctx_state { PENDING, DONE, CANCELED };

struct em_proxying_ctx {
  // The user-provided function and argument.
  void (*func)(em_proxying_ctx*, void*);
  void* arg;

  enum ctx_kind kind;
  union {
    // Context for synchronous proxying.
    struct {
      // Update `state` and signal the condition variable once the proxied task
      // is done or canceled.
      enum ctx_state state;
      pthread_mutex_t mutex;
      pthread_cond_t cond;
    } sync;

    // Context for proxying with callbacks.
    struct {
      em_proxying_queue* queue;
      pthread_t caller_thread;
      void (*callback)(void*);
      void (*cancel)(void*);
    } cb;
  };

  // A doubly linked list of contexts associated with active work on a single
  // thread. If the thread is canceled, it will traverse this list to find
  // contexts that need to be canceled.
  struct em_proxying_ctx* next;
  struct em_proxying_ctx* prev;
};

// The key that `cancel_active_ctxs` is bound to so that it runs when a thread
// is canceled or exits.
static pthread_key_t active_ctxs;
static pthread_once_t active_ctxs_once = PTHREAD_ONCE_INIT;

static void cancel_ctx(void* arg);
static void cancel_active_ctxs(void* arg);

static void init_active_ctxs(void) {
  int ret = pthread_key_create(&active_ctxs, cancel_active_ctxs);
  assert(ret == 0);
  (void)ret;
}

static void add_active_ctx(em_proxying_ctx* ctx) {
  assert(ctx != NULL);
  em_proxying_ctx* head = pthread_getspecific(active_ctxs);
  if (head == NULL) {
    // This is the only active context; initialize the active contexts list.
    ctx->next = ctx->prev = ctx;
    pthread_setspecific(active_ctxs, ctx);
  } else {
    // Insert this context at the tail of the list just before `head`.
    ctx->next = head;
    ctx->prev = head->prev;
    ctx->next->prev = ctx;
    ctx->prev->next = ctx;
  }
}

static void remove_active_ctx(em_proxying_ctx* ctx) {
  assert(ctx != NULL);
  assert(ctx->next != NULL);
  assert(ctx->prev != NULL);
  if (ctx->next == ctx) {
    // This is the only active context; clear the active contexts list.
    ctx->next = ctx->prev = NULL;
    pthread_setspecific(active_ctxs, NULL);
    return;
  }

  // Update the list head if we are removing the current head.
  em_proxying_ctx* head = pthread_getspecific(active_ctxs);
  if (ctx == head) {
    pthread_setspecific(active_ctxs, head->next);
  }

  // Remove the context from the list.
  ctx->prev->next = ctx->next;
  ctx->next->prev = ctx->prev;
  ctx->next = ctx->prev = NULL;
}

static void cancel_active_ctxs(void* arg) {
  pthread_setspecific(active_ctxs, NULL);
  em_proxying_ctx* head = arg;
  em_proxying_ctx* curr = head;
  do {
    em_proxying_ctx* next = curr->next;
    cancel_ctx(curr);
    curr = next;
  } while (curr != head);
}

static void em_proxying_ctx_init_sync(em_proxying_ctx* ctx,
                                      void (*func)(em_proxying_ctx*, void*),
                                      void* arg) {
  pthread_once(&active_ctxs_once, init_active_ctxs);
  *ctx = (em_proxying_ctx){
    .func = func,
    .arg = arg,
    .kind = SYNC,
    .sync =
      {
        .state = PENDING,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
      },
  };
}

static void em_proxying_ctx_init_callback(em_proxying_ctx* ctx,
                                          em_proxying_queue* queue,
                                          pthread_t caller_thread,
                                          void (*func)(em_proxying_ctx*, void*),
                                          void (*callback)(void*),
                                          void (*cancel)(void*),
                                          void* arg) {
  pthread_once(&active_ctxs_once, init_active_ctxs);
  *ctx = (em_proxying_ctx){
    .func = func,
    .arg = arg,
    .kind = CALLBACK,
    .cb =
      {
        .queue = queue,
        .caller_thread = caller_thread,
        .callback = callback,
        .cancel = cancel,
      },
  };
}

static void em_proxying_ctx_deinit(em_proxying_ctx* ctx) {
  if (ctx->kind == SYNC) {
    pthread_mutex_destroy(&ctx->sync.mutex);
    pthread_cond_destroy(&ctx->sync.cond);
  }
  // TODO: We should probably have some kind of refcounting scheme to keep
  // `queue` alive for callback ctxs.
}

static void free_ctx(void* arg) {
  em_proxying_ctx* ctx = arg;
  em_proxying_ctx_deinit(ctx);
  free(ctx);
}

// Free the callback info on the same thread it was originally allocated on.
// This may be more efficient.
static void call_callback_then_free_ctx(void* arg) {
  em_proxying_ctx* ctx = arg;
  ctx->cb.callback(ctx->arg);
  free_ctx(ctx);
}

void emscripten_proxy_finish(em_proxying_ctx* ctx) {
  if (ctx->kind == SYNC) {
    pthread_mutex_lock(&ctx->sync.mutex);
    ctx->sync.state = DONE;
    remove_active_ctx(ctx);
    pthread_mutex_unlock(&ctx->sync.mutex);
    pthread_cond_signal(&ctx->sync.cond);
  } else {
    // Schedule the callback on the caller thread. If the caller thread has
    // already died or dies before the callback is executed, then at least make
    // sure the context is freed.
    remove_active_ctx(ctx);
    if (!do_proxy(ctx->cb.queue,
                  ctx->cb.caller_thread,
                  (task){call_callback_then_free_ctx, free_ctx, ctx})) {
      free_ctx(ctx);
    }
  }
}

static void call_cancel_then_free_ctx(void* arg) {
  em_proxying_ctx* ctx = arg;
  ctx->cb.cancel(ctx->arg);
  free_ctx(ctx);
}

static void cancel_ctx(void* arg) {
  em_proxying_ctx* ctx = arg;
  if (ctx->kind == SYNC) {
    pthread_mutex_lock(&ctx->sync.mutex);
    ctx->sync.state = CANCELED;
    pthread_mutex_unlock(&ctx->sync.mutex);
    pthread_cond_signal(&ctx->sync.cond);
  } else {
    if (ctx->cb.cancel == NULL ||
        !do_proxy(ctx->cb.queue,
                  ctx->cb.caller_thread,
                  (task){call_cancel_then_free_ctx, free_ctx, ctx})) {
      free_ctx(ctx);
    }
  }
}

// Helper for wrapping the call with ctx as a `void (*)(void*)`.
static void call_with_ctx(void* arg) {
  em_proxying_ctx* ctx = arg;
  add_active_ctx(ctx);
  ctx->func(ctx, ctx->arg);
}

int emscripten_proxy_sync_with_ctx(em_proxying_queue* q,
                                   pthread_t target_thread,
                                   void (*func)(em_proxying_ctx*, void*),
                                   void* arg) {
  assert(!pthread_equal(target_thread, pthread_self()) &&
         "Cannot synchronously wait for work proxied to the current thread");
  em_proxying_ctx ctx;
  em_proxying_ctx_init_sync(&ctx, func, arg);
  if (!do_proxy(q, target_thread, (task){call_with_ctx, cancel_ctx, &ctx})) {
    em_proxying_ctx_deinit(&ctx);
    return 0;
  }
  pthread_mutex_lock(&ctx.sync.mutex);
  while (ctx.sync.state == PENDING) {
    pthread_cond_wait(&ctx.sync.cond, &ctx.sync.mutex);
  }
  pthread_mutex_unlock(&ctx.sync.mutex);
  int ret = ctx.sync.state == DONE;
  em_proxying_ctx_deinit(&ctx);
  return ret;
}

// Helper for signaling the end of the task after the user function returns.
static void call_then_finish_task(em_proxying_ctx* ctx, void* arg) {
  task* t = arg;
  t->func(t->arg);
  emscripten_proxy_finish(ctx);
}

int emscripten_proxy_sync(em_proxying_queue* q,
                          pthread_t target_thread,
                          void (*func)(void*),
                          void* arg) {
  task t = {.func = func, .arg = arg};
  return emscripten_proxy_sync_with_ctx(
    q, target_thread, call_then_finish_task, &t);
}

static int do_proxy_callback(em_proxying_queue* q,
                             pthread_t target_thread,
                             void (*func)(em_proxying_ctx* ctx, void*),
                             void (*callback)(void*),
                             void (*cancel)(void*),
                             void* arg,
                             em_proxying_ctx* ctx) {
  em_proxying_ctx_init_callback(
    ctx, q, pthread_self(), func, callback, cancel, arg);
  if (!do_proxy(q, target_thread, (task){call_with_ctx, cancel_ctx, ctx})) {
    free_ctx(ctx);
    return 0;
  }
  return 1;
}

int emscripten_proxy_callback_with_ctx(em_proxying_queue* q,
                                       pthread_t target_thread,
                                       void (*func)(em_proxying_ctx* ctx,
                                                    void*),
                                       void (*callback)(void*),
                                       void (*cancel)(void*),
                                       void* arg) {
  em_proxying_ctx* ctx = malloc(sizeof(*ctx));
  if (ctx == NULL) {
    return 0;
  }
  return do_proxy_callback(q, target_thread, func, callback, cancel, arg, ctx);
}

typedef struct callback_ctx {
  void (*func)(void*);
  void (*callback)(void*);
  void (*cancel)(void*);
  void* arg;
} callback_ctx;

static void call_then_finish_callback(em_proxying_ctx* ctx, void* arg) {
  callback_ctx* cb_ctx = arg;
  cb_ctx->func(cb_ctx->arg);
  emscripten_proxy_finish(ctx);
}

static void callback_call(void* arg) {
  callback_ctx* cb_ctx = arg;
  cb_ctx->callback(cb_ctx->arg);
}

static void callback_cancel(void* arg) {
  callback_ctx* cb_ctx = arg;
  if (cb_ctx->cancel != NULL) {
    cb_ctx->cancel(cb_ctx->arg);
  }
}

int emscripten_proxy_callback(em_proxying_queue* q,
                              pthread_t target_thread,
                              void (*func)(void*),
                              void (*callback)(void*),
                              void (*cancel)(void*),
                              void* arg) {
  // Allocate the em_proxying_ctx and the user ctx as a single block that will
  // be freed when the `em_proxying_ctx` is freed.
  struct block {
    em_proxying_ctx ctx;
    callback_ctx cb_ctx;
  };
  struct block* block = malloc(sizeof(*block));
  if (block == NULL) {
    return 0;
  }
  block->cb_ctx = (callback_ctx){func, callback, cancel, arg};
  return do_proxy_callback(q,
                           target_thread,
                           call_then_finish_callback,
                           callback_call,
                           callback_cancel,
                           &block->cb_ctx,
                           &block->ctx);
}

typedef struct promise_ctx {
  void (*func)(em_proxying_ctx*, void*);
  void* arg;
  em_promise_t promise;
} promise_ctx;

static void promise_call(em_proxying_ctx* ctx, void* arg) {
  promise_ctx* promise_ctx = arg;
  promise_ctx->func(ctx, promise_ctx->arg);
}

static void promise_fulfill(void* arg) {
  promise_ctx* promise_ctx = arg;
  emscripten_promise_resolve(promise_ctx->promise, EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(promise_ctx->promise);
}

static void promise_reject(void* arg) {
  promise_ctx* promise_ctx = arg;
  emscripten_promise_resolve(promise_ctx->promise, EM_PROMISE_REJECT, NULL);
  emscripten_promise_destroy(promise_ctx->promise);
}

static em_promise_t do_proxy_promise(em_proxying_queue* q,
                                     pthread_t target_thread,
                                     void (*func)(em_proxying_ctx*, void*),
                                     void* arg,
                                     em_promise_t promise,
                                     em_proxying_ctx* ctx,
                                     promise_ctx* promise_ctx) {
  *promise_ctx = (struct promise_ctx){func, arg, promise};
  if (!do_proxy_callback(q,
                         target_thread,
                         promise_call,
                         promise_fulfill,
                         promise_reject,
                         promise_ctx,
                         ctx)) {
    emscripten_promise_resolve(promise, EM_PROMISE_REJECT, NULL);
    return promise;
  }
  // Return a separate promise to ensure that the internal promise will stay
  // alive until the callbacks are called.
  em_promise_t ret = emscripten_promise_create();
  emscripten_promise_resolve(ret, EM_PROMISE_MATCH, promise);
  return ret;
}

em_promise_t emscripten_proxy_promise_with_ctx(em_proxying_queue* q,
                                               pthread_t target_thread,
                                               void (*func)(em_proxying_ctx*,
                                                            void*),
                                               void* arg) {
  em_promise_t promise = emscripten_promise_create();
  // Allocate the em_proxying_ctx and promise ctx as a single block that will be
  // freed when the `em_proxying_ctx` is freed.
  struct block {
    em_proxying_ctx ctx;
    promise_ctx promise_ctx;
  };
  struct block* block = malloc(sizeof(*block));
  if (block == NULL) {
    emscripten_promise_resolve(promise, EM_PROMISE_REJECT, NULL);
    return promise;
  }
  return do_proxy_promise(
    q, target_thread, func, arg, promise, &block->ctx, &block->promise_ctx);
}

em_promise_t emscripten_proxy_promise(em_proxying_queue* q,
                                      pthread_t target_thread,
                                      void (*func)(void*),
                                      void* arg) {
  em_promise_t promise = emscripten_promise_create();
  // Allocate the em_proxying_ctx, promise ctx, and user task as a single block
  // that will be freed when the `em_proxying_ctx` is freed.
  struct block {
    em_proxying_ctx ctx;
    promise_ctx promise_ctx;
    task task;
  };
  struct block* block = malloc(sizeof(*block));
  if (block == NULL) {
    emscripten_promise_resolve(promise, EM_PROMISE_REJECT, NULL);
    return promise;
  }
  block->task = (task){.func = func, .arg = arg};
  return do_proxy_promise(q,
                          target_thread,
                          call_then_finish_task,
                          &block->task,
                          promise,
                          &block->ctx,
                          &block->promise_ctx);
}

typedef struct proxied_js_func_t {
  int funcIndex;
  void* emAsmAddr;
  pthread_t callingThread;
  int numArgs;
  double* argBuffer;
  double result;
  bool owned;
} proxied_js_func_t;

static void run_js_func(void* arg) {
  proxied_js_func_t* f = (proxied_js_func_t*)arg;
  f->result = _emscripten_receive_on_main_thread_js(
    f->funcIndex, f->emAsmAddr, f->callingThread, f->numArgs, f->argBuffer);
  if (f->owned) {
    free(f->argBuffer);
    free(f);
  }
}

double _emscripten_run_on_main_thread_js(int func_index,
                                         void* em_asm_addr,
                                         int num_args,
                                         double* buffer,
                                         int sync) {
  proxied_js_func_t f = {
    .funcIndex = func_index,
    .emAsmAddr = em_asm_addr,
    .callingThread = pthread_self(),
    .numArgs = num_args,
    .argBuffer = buffer,
    .owned = false,
  };

  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  pthread_t target = emscripten_main_runtime_thread_id();

  if (sync) {
    if (!emscripten_proxy_sync(q, target, run_js_func, &f)) {
      assert(false && "emscripten_proxy_sync failed");
      return 0;
    }
    return f.result;
  }

  // Make a heap-heap allocated copy of the proxied_js_func_t
  proxied_js_func_t* arg = malloc(sizeof(proxied_js_func_t));
  *arg = f;
  arg->owned = true;

  // Also make a copyh of the argBuffer.
  arg->argBuffer = malloc(num_args*sizeof(double));
  memcpy(arg->argBuffer, buffer, num_args*sizeof(double));

  if (!emscripten_proxy_async(q, target, run_js_func, arg)) {
    assert(false && "emscripten_proxy_async failed");
  }
  return 0;
}
