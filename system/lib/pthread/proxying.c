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
#include <stdlib.h>
#include <string.h>

#include "em_task_queue.h"
#include "thread_mailbox.h"

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

enum ctx_state { PENDING, DONE, CANCELED };

struct em_proxying_ctx {
  // The user-provided function and argument.
  void (*func)(em_proxying_ctx*, void*);
  void* arg;
  // Update `state` and signal the condition variable once the proxied task is
  // done or canceled.
  enum ctx_state state;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  // A doubly linked list of contexts associated with active work on a single
  // thread. If the thread is canceled, it will traverse this list to find
  // contexts that need to be canceled.
  struct em_proxying_ctx* next;
  struct em_proxying_ctx* prev;
};

// The key that `cancel_active_ctxs` is bound to so that it runs when a thread
// is canceled or exits.
static pthread_key_t active_ctxs;

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

static void em_proxying_ctx_init(em_proxying_ctx* ctx,
                                 void (*func)(em_proxying_ctx*, void*),
                                 void* arg) {
  static pthread_once_t once = PTHREAD_ONCE_INIT;
  pthread_once(&once, init_active_ctxs);
  *ctx = (em_proxying_ctx){.func = func,
                           .arg = arg,
                           .state = PENDING,
                           .mutex = PTHREAD_MUTEX_INITIALIZER,
                           .cond = PTHREAD_COND_INITIALIZER};
}

static void em_proxying_ctx_deinit(em_proxying_ctx* ctx) {
  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->cond);
}

void emscripten_proxy_finish(em_proxying_ctx* ctx) {
  pthread_mutex_lock(&ctx->mutex);
  ctx->state = DONE;
  remove_active_ctx(ctx);
  pthread_mutex_unlock(&ctx->mutex);
  pthread_cond_signal(&ctx->cond);
}

static void cancel_ctx(void* arg) {
  em_proxying_ctx* ctx = arg;
  pthread_mutex_lock(&ctx->mutex);
  ctx->state = CANCELED;
  pthread_mutex_unlock(&ctx->mutex);
  pthread_cond_signal(&ctx->cond);
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
  em_proxying_ctx_init(&ctx, func, arg);
  if (!do_proxy(q, target_thread, (task){call_with_ctx, cancel_ctx, &ctx})) {
    em_proxying_ctx_deinit(&ctx);
    return 0;
  }
  pthread_mutex_lock(&ctx.mutex);
  while (ctx.state == PENDING) {
    pthread_cond_wait(&ctx.cond, &ctx.mutex);
  }
  pthread_mutex_unlock(&ctx.mutex);
  int ret = ctx.state == DONE;
  em_proxying_ctx_deinit(&ctx);
  return ret;
}

// Helper for signaling the end of the task after the user function returns.
static void call_then_finish(em_proxying_ctx* ctx, void* arg) {
  task* t = arg;
  t->func(t->arg);
  emscripten_proxy_finish(ctx);
}

int emscripten_proxy_sync(em_proxying_queue* q,
                          pthread_t target_thread,
                          void (*func)(void*),
                          void* arg) {
  task t = {.func = func, .arg = arg};
  return emscripten_proxy_sync_with_ctx(q, target_thread, call_then_finish, &t);
}

// Helper struct for organizing a proxied call and its callback on the original
// thread.
struct callback {
  em_proxying_queue* q;
  pthread_t caller_thread;
  void* (*func)(void*);
  void* arg;
  void (*callback)(void* arg, void* result);
  void* callback_arg;
  void* result;
};

// Free the callback info on the same thread it was originally allocated on.
// This may be more efficient.
static void call_callback_then_free(void* arg) {
  struct callback* info = (struct callback*)arg;
  info->callback(info->callback_arg, info->result);
  free(arg);
}

static void call_then_schedule_callback(void* arg) {
  struct callback* info = (struct callback*)arg;
  info->result = info->func(info->arg);
  if (!emscripten_proxy_async(
        info->q, info->caller_thread, call_callback_then_free, arg)) {
    // No way to gracefully report that we failed to schedule the callback, so
    // abort.
    abort();
  }
}

int emscripten_proxy_async_with_callback(em_proxying_queue* q,
                                         pthread_t target_thread,
                                         void* (*func)(void*),
                                         void* arg,
                                         void (*callback)(void* arg,
                                                          void* result),
                                         void* callback_arg) {
  struct callback* info = malloc(sizeof(*info));
  if (info == NULL) {
    return 0;
  }
  *info = (struct callback){
    .q = q,
    .caller_thread = pthread_self(),
    .func = func,
    .arg = arg,
    .callback = callback,
    .callback_arg = callback_arg,
    .result = NULL,
  };
  return emscripten_proxy_async(
    q, target_thread, call_then_schedule_callback, info);
}
