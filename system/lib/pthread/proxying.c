/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "proxying.h"

#define TASK_QUEUE_INITIAL_CAPACITY 128

extern int _emscripten_notify_proxying_queue(pthread_t target_thread,
                                             pthread_t curr_thread,
                                             pthread_t main_thread,
                                             em_proxying_queue* queue);

typedef struct task {
  void (*func)(void*);
  void* arg;
} task;

// A task queue for a particular thread. Organized into a linked list of
// task_queues for different threads.
typedef struct task_queue {
  // The target thread for this task_queue.
  pthread_t thread;
  // Recursion guard. TODO: We disallow recursive processing because that's what
  // the old proxying API does, so it is safer to start with the same behavior.
  // Experiment with relaxing this restriction once the old API uses these
  // queues as well.
  int processing;
  // Ring buffer of tasks of size `capacity`. New tasks are enqueued at
  // `tail` and dequeued at `head`.
  task* tasks;
  int capacity;
  int head;
  int tail;
} task_queue;

static int task_queue_init(task_queue* tasks, pthread_t thread) {
  task* task_buffer = malloc(sizeof(task) * TASK_QUEUE_INITIAL_CAPACITY);
  if (task_buffer == NULL) {
    return 0;
  }
  *tasks = (task_queue){.thread = thread,
                        .processing = 0,
                        .tasks = task_buffer,
                        .capacity = TASK_QUEUE_INITIAL_CAPACITY,
                        .head = 0,
                        .tail = 0};
  return 1;
}

static void task_queue_deinit(task_queue* tasks) { free(tasks->tasks); }

// Not thread safe.
static int task_queue_is_empty(task_queue* tasks) {
  return tasks->head == tasks->tail;
}

// Not thread safe.
static int task_queue_full(task_queue* tasks) {
  return tasks->head == (tasks->tail + 1) % tasks->capacity;
}

// // Not thread safe. Returns 1 on success and 0 on failure.
static int task_queue_grow(task_queue* tasks) {
  // Allocate a larger task queue.
  int new_capacity = tasks->capacity * 2;
  task* new_tasks = malloc(sizeof(task) * new_capacity);
  if (new_tasks == NULL) {
    return 0;
  }
  // Copy the tasks such that the head of the queue is at the beginning of the
  // buffer. There are two cases to handle: either the queue wraps around the
  // end of the old buffer or it does not.
  int queued_tasks;
  if (tasks->head <= tasks->tail) {
    // No wrap. Copy the tasks in one chunk.
    queued_tasks = tasks->tail - tasks->head;
    memcpy(new_tasks, &tasks->tasks[tasks->head], sizeof(task) * queued_tasks);
  } else {
    // Wrap. Copy `first_queued` tasks up to the end of the old buffer and
    // `last_queued` tasks at the beginning of the old buffer.
    int first_queued = tasks->capacity - tasks->head;
    int last_queued = tasks->tail;
    queued_tasks = first_queued + last_queued;
    memcpy(new_tasks, &tasks->tasks[tasks->head], sizeof(task) * first_queued);
    memcpy(new_tasks + first_queued, tasks->tasks, sizeof(task) * last_queued);
  }
  free(tasks->tasks);
  tasks->tasks = new_tasks;
  tasks->capacity = new_capacity;
  tasks->head = 0;
  tasks->tail = queued_tasks;
  return 1;
}

// Not thread safe. Returns 1 on success and 0 on failure.
static int task_queue_enqueue(task_queue* tasks, task t) {
  if (task_queue_full(tasks) && !task_queue_grow(tasks)) {
    return 0;
  }
  tasks->tasks[tasks->tail] = t;
  tasks->tail = (tasks->tail + 1) % tasks->capacity;
  return 1;
}

// Not thread safe. Assumes the queue is not empty.
static task task_queue_dequeue(task_queue* tasks) {
  task t = tasks->tasks[tasks->head];
  tasks->head = (tasks->head + 1) % tasks->capacity;
  return t;
}

struct em_proxying_queue {
  // Protects all accesses to all task_queues.
  pthread_mutex_t mutex;
  // `size` task queues stored in an array of size `capacity`.
  task_queue* task_queues;
  int size;
  int capacity;
};

static em_proxying_queue system_proxying_queue = {.mutex =
                                                    PTHREAD_MUTEX_INITIALIZER,
                                                  .task_queues = NULL,
                                                  .size = 0,
                                                  .capacity = 0};

em_proxying_queue* emscripten_proxy_get_system_queue(void) {
  return &system_proxying_queue;
}

em_proxying_queue* em_proxying_queue_create(void) {
  em_proxying_queue* q = malloc(sizeof(em_proxying_queue));
  if (q == NULL) {
    return NULL;
  }
  *q = (em_proxying_queue){.mutex = PTHREAD_MUTEX_INITIALIZER,
                           .task_queues = NULL,
                           .size = 0,
                           .capacity = 0};
  return q;
}

void em_proxying_queue_destroy(em_proxying_queue* q) {
  assert(q != NULL);
  assert(q != &system_proxying_queue && "cannot destroy system proxying queue");
  // No need to acquire the lock; no one should be racing with the destruction
  // of the queue.
  pthread_mutex_destroy(&q->mutex);
  for (int i = 0; i < q->size; i++) {
    task_queue_deinit(&q->task_queues[i]);
  }
  free(q->task_queues);
  free(q);
}

// Not thread safe. Returns -1 if there are no tasks for the thread.
static int get_tasks_index_for_thread(em_proxying_queue* q, pthread_t thread) {
  assert(q != NULL);
  for (int i = 0; i < q->size; i++) {
    if (pthread_equal(q->task_queues[i].thread, thread)) {
      return i;
    }
  }
  return -1;
}

// Not thread safe.
static task_queue* get_or_add_tasks_for_thread(em_proxying_queue* q,
                                               pthread_t thread) {
  int tasks_index = get_tasks_index_for_thread(q, thread);
  if (tasks_index != -1) {
    return &q->task_queues[tasks_index];
  }
  // There were no tasks for the thread; initialize a new task_queue. If there
  // are not enough queues, allocate more.
  if (q->size == q->capacity) {
    int new_capacity = q->capacity == 0 ? 1 : q->capacity * 2;
    task_queue* new_task_queues =
      realloc(q->task_queues, sizeof(task_queue) * new_capacity);
    if (new_task_queues == NULL) {
      return NULL;
    }
    q->task_queues = new_task_queues;
    q->capacity = new_capacity;
  }
  // Initialize the next available task queue.
  task_queue* tasks = &q->task_queues[q->size];
  if (!task_queue_init(tasks, thread)) {
    return NULL;
  }
  q->size++;
  return tasks;
}

// Exported for use in worker.js.
EMSCRIPTEN_KEEPALIVE
void emscripten_proxy_execute_queue(em_proxying_queue* q) {
  assert(q != NULL);
  pthread_mutex_lock(&q->mutex);
  int tasks_index = get_tasks_index_for_thread(q, pthread_self());
  task_queue* tasks = tasks_index == -1 ? NULL : &q->task_queues[tasks_index];
  if (tasks == NULL || tasks->processing) {
    // No tasks for this thread or they are already being processed.
    pthread_mutex_unlock(&q->mutex);
    return;
  }
  // Found the task queue; process the tasks.
  tasks->processing = 1;
  while (!task_queue_is_empty(tasks)) {
    task t = task_queue_dequeue(tasks);
    // Unlock while the task is running to allow more work to be queued in
    // parallel.
    pthread_mutex_unlock(&q->mutex);
    t.func(t.arg);
    pthread_mutex_lock(&q->mutex);
    // The tasks might have been reallocated, so recalculate the pointer.
    tasks = &q->task_queues[tasks_index];
  }
  tasks->processing = 0;
  pthread_mutex_unlock(&q->mutex);
}

int emscripten_proxy_async(em_proxying_queue* q,
                           pthread_t target_thread,
                           void (*func)(void*),
                           void* arg) {
  assert(q != NULL);
  pthread_mutex_lock(&q->mutex);
  task_queue* tasks = get_or_add_tasks_for_thread(q, target_thread);
  if (tasks == NULL) {
    goto failed;
  }
  int empty = task_queue_is_empty(tasks);
  if (!task_queue_enqueue(tasks, (task){func, arg})) {
    goto failed;
  }
  pthread_mutex_unlock(&q->mutex);
  // If the queue was previously empty, notify the target thread to process it.
  // Otherwise, the target thread was already notified when the existing work
  // was enqueued so we don't need to notify it again.
  if (empty) {
    _emscripten_notify_proxying_queue(
      target_thread, pthread_self(), emscripten_main_browser_thread_id(), q);
  }
  return 1;

failed:
  pthread_mutex_unlock(&q->mutex);
  return 0;
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
