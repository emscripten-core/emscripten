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
#include <stdlib.h>
#include <string.h>

#include "em_task_queue.h"
#include "proxying_notification_state.h"

// Proxy Queue Lifetime Management
// -------------------------------
//
// Proxied tasks are executed either when the user manually calls
// `emscripten_proxy_execute_queue` on the target thread or when the target
// thread returns to the event loop. The queue does not know which execution
// path will be used ahead of time when the work is proxied, so it must
// conservatively send a message to the target thread's event loop in case the
// user expects the event loop to drive the execution. These notifications
// contain references to the queue that will be dereferenced when the target
// thread returns to its event loop and receives the notification, even if the
// user manages the execution of the queue themselves.
//
// To avoid use-after-free bugs, we cannot free a queue immediately when a user
// calls `em_proxying_queue_destroy`; instead, we have to defer freeing the
// queue until all of its outstanding notifications have been processed. We
// defer freeing the queue using a reference counting scheme. Each time a
// notification containing a reference to the a thread-local task queue is
// generated, we set a flag on that task queue. Each time that task queue is
// processed, we clear the flag. The proxying queue can only be freed once
// `em_proxying_queue_destroy` has been called and the notification flags on
// each of its task queues have been cleared.
//
// But an extra complication is that the target thread may have died by the time
// it gets back to its event loop to process its notifications. This can happen
// when a user proxies some work to a thread, then calls
// `emscripten_proxy_execute_queue` on that thread, then destroys the queue and
// exits the thread. In that situation no work will be dropped, but the thread's
// worker will still receive a notification and have to clear the notification
// flag without a live runtime. Without a live runtime, there is no stack, so
// the worker cannot safely free the queue at this point even if the refcount
// goes to zero. We need a separate thread with a live runtime to perform the
// free.
//
// To ensure that queues are eventually freed, we place destroyed queues in a
// global "zombie list" where they wait for their notification flags to be
// cleared. The zombie list is scanned whenever a new queue is constructed and
// any of the zombie queues without outstanding notifications are freed. In
// principle the zombie list could be scanned at any time, but the queue
// constructor is a nice place to do it because scanning there is sufficient to
// keep the number of zombie queues from growing without bound; creating a new
// zombie ultimately requires creating a new queue.
//
// -------------------------------

struct em_proxying_queue {
  // Protects all accesses to em_task_queues, size, and capacity.
  pthread_mutex_t mutex;
  // `size` task queue pointers stored in an array of size `capacity`.
  em_task_queue** task_queues;
  int size;
  int capacity;
  // Doubly linked list pointers for the zombie list.
  em_proxying_queue* zombie_prev;
  em_proxying_queue* zombie_next;
};

// The system proxying queue.
static em_proxying_queue system_proxying_queue = {.mutex =
                                                    PTHREAD_MUTEX_INITIALIZER,
                                                  .task_queues = NULL,
                                                  .size = 0,
                                                  .capacity = 0,
                                                  .zombie_prev = NULL,
                                                  .zombie_next = NULL};

em_proxying_queue* emscripten_proxy_get_system_queue(void) {
  return &system_proxying_queue;
}

// The head of the zombie list. Its mutex protects access to the list and its
// other fields are not used.
static em_proxying_queue zombie_list_head = {.mutex = PTHREAD_MUTEX_INITIALIZER,
                                             .zombie_prev = &zombie_list_head,
                                             .zombie_next = &zombie_list_head};

static void em_proxying_queue_free(em_proxying_queue* q) {
  pthread_mutex_destroy(&q->mutex);
  for (int i = 0; i < q->size; i++) {
    em_task_queue_destroy(q->task_queues[i]);
  }
  free(q->task_queues);
  free(q);
}

// Does not lock `q` because it should only be called after `q` has been
// destroyed when it would be UB for new work to come in and race to generate a
// new notification.
static int has_notification(em_proxying_queue* q) {
  for (int i = 0; i < q->size; i++) {
    if (q->task_queues[i]->notification != NOTIFICATION_NONE) {
      return 1;
    }
  }
  return 0;
}

static void cull_zombies() {
  pthread_mutex_lock(&zombie_list_head.mutex);
  em_proxying_queue* curr = zombie_list_head.zombie_next;
  while (curr != &zombie_list_head) {
    em_proxying_queue* next = curr->zombie_next;
    if (!has_notification(curr)) {
      // Remove the zombie from the list and free it.
      curr->zombie_prev->zombie_next = curr->zombie_next;
      curr->zombie_next->zombie_prev = curr->zombie_prev;
      em_proxying_queue_free(curr);
    }
    curr = next;
  }
  pthread_mutex_unlock(&zombie_list_head.mutex);
}

em_proxying_queue* em_proxying_queue_create(void) {
  // Free any queue that has been destroyed and is safe to free.
  cull_zombies();

  // Allocate the new queue.
  em_proxying_queue* q = malloc(sizeof(em_proxying_queue));
  if (q == NULL) {
    return NULL;
  }
  *q = (em_proxying_queue){.mutex = PTHREAD_MUTEX_INITIALIZER,
                           .task_queues = NULL,
                           .size = 0,
                           .capacity = 0,
                           .zombie_prev = NULL,
                           .zombie_next = NULL};
  return q;
}

void em_proxying_queue_destroy(em_proxying_queue* q) {
  assert(q != NULL);
  assert(q != &system_proxying_queue && "cannot destroy system proxying queue");
  assert(!q->zombie_next && !q->zombie_prev &&
         "double freeing em_proxying_queue!");
  if (!has_notification(q)) {
    // No outstanding references to the queue, so we can go ahead and free it.
    em_proxying_queue_free(q);
    return;
  }
  // Otherwise add the queue to the zombie list so that it will eventually be
  // freed safely.
  pthread_mutex_lock(&zombie_list_head.mutex);
  q->zombie_next = zombie_list_head.zombie_next;
  q->zombie_prev = &zombie_list_head;
  q->zombie_next->zombie_prev = q;
  q->zombie_prev->zombie_next = q;
  pthread_mutex_unlock(&zombie_list_head.mutex);
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

// Exported for use in worker.js, but otherwise an internal function.
EMSCRIPTEN_KEEPALIVE
void _emscripten_proxy_execute_task_queue(em_task_queue* tasks) {
  // Before we attempt to execute a request from another thread make sure we
  // are in sync with all the loaded code.
  // For example, in PROXY_TO_PTHREAD the atexit functions are called via
  // a proxied call, and without this call to syncronize we would crash if
  // any atexit functions were registered from a side module.
  em_task_queue_execute(tasks);
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

int emscripten_proxy_async(em_proxying_queue* q,
                           pthread_t target_thread,
                           void (*func)(void*),
                           void* arg) {
  assert(q != NULL);
  pthread_mutex_lock(&q->mutex);
  em_task_queue* tasks = get_or_add_tasks_for_thread(q, target_thread);
  pthread_mutex_unlock(&q->mutex);
  if (tasks == NULL) {
    return 0;
  }
  pthread_mutex_lock(&tasks->mutex);
  int enqueued = em_task_queue_enqueue(tasks, (task){func, arg});
  pthread_mutex_unlock(&tasks->mutex);
  if (!enqueued) {
    return 0;
  }

  em_task_queue_notify(tasks);
  return 1;
}

struct em_proxying_ctx {
  // The user-provided function and argument.
  void (*func)(em_proxying_ctx*, void*);
  void* arg;
  // Set `done` to 1 and signal the condition variable once the proxied task is
  // done.
  int done;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

static void em_proxying_ctx_init(em_proxying_ctx* ctx,
                                 void (*func)(em_proxying_ctx*, void*),
                                 void* arg) {
  *ctx = (em_proxying_ctx){.func = func,
                           .arg = arg,
                           .done = 0,
                           .mutex = PTHREAD_MUTEX_INITIALIZER,
                           .cond = PTHREAD_COND_INITIALIZER};
}

static void em_proxying_ctx_deinit(em_proxying_ctx* ctx) {
  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->cond);
}

void emscripten_proxy_finish(em_proxying_ctx* ctx) {
  pthread_mutex_lock(&ctx->mutex);
  ctx->done = 1;
  pthread_mutex_unlock(&ctx->mutex);
  pthread_cond_signal(&ctx->cond);
}

// Helper for wrapping the call with ctx as a `void (*)(void*)`.
static void call_with_ctx(void* p) {
  em_proxying_ctx* ctx = (em_proxying_ctx*)p;
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
  if (!emscripten_proxy_async(q, target_thread, call_with_ctx, &ctx)) {
    return 0;
  }
  pthread_mutex_lock(&ctx.mutex);
  while (!ctx.done) {
    pthread_cond_wait(&ctx.cond, &ctx.mutex);
  }
  pthread_mutex_unlock(&ctx.mutex);
  em_proxying_ctx_deinit(&ctx);
  return 1;
}

// Helper for signaling the end of the task after the user function returns.
static void call_then_finish(em_proxying_ctx* ctx, void* arg) {
  task* t = (task*)arg;
  t->func(t->arg);
  emscripten_proxy_finish(ctx);
}

int emscripten_proxy_sync(em_proxying_queue* q,
                          pthread_t target_thread,
                          void (*func)(void*),
                          void* arg) {
  task t = {func, arg};
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
