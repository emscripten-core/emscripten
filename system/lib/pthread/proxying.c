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
// notification containing a reference to the queue is generated, we increase
// the reference count and each time one of the notifications is received and
// processed, we decrease the reference count. The queue can only be freed once
// `em_proxying_queue_destroy` has been called and the reference count has
// reached zero.
//
// But an extra complication is that the target thread may have died by the time
// it gets back to its event loop to process its notifications. This can happen
// when a user proxies some work to a thread, then calls
// `emscripten_proxy_execute_queue` on that thread, then destroys the queue and
// exits the thread. In that situation no work will be dropped, but the thread's
// worker will still receive a notification and have to decrease the reference
// count without a live runtime. Without a live runtime, there is no stack, so
// the worker cannot safely free the queue at this point even if the refcount
// goes to zero. We need a separate thread with a live runtime to perform the
// free.
//
// To ensure that queues are eventually freed, we place destroyed queues in a
// global "zombie list" where they wait for their refcounts to fall to zero. The
// zombie list is scanned whenever a new queue is constructed and any of the
// zombie queues with zero refcounts are freed. In principle the zombie list
// could be scanned at any time, but the queue constructor is a nice place to do
// it because scanning there is sufficient to keep the number of zombie queues
// from growing without bound; creating a new zombie ultimately requires
// creating a new queue.

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
  // The number of references to this queue that exist in JS event queues.
  // Decremented directly from JS, so this must be the first field.
  _Atomic int js_refcount;
  // Doubly linked list pointers for the zombie list.
  em_proxying_queue* zombie_prev;
  em_proxying_queue* zombie_next;
  // Protects all accesses to task_queues, size, and capacity.
  pthread_mutex_t mutex;
  // `size` task queues stored in an array of size `capacity`.
  task_queue* task_queues;
  int size;
  int capacity;
};

// The system proxying queue.
static em_proxying_queue system_proxying_queue = {.js_refcount = 0,
                                                  .zombie_prev = NULL,
                                                  .zombie_next = NULL,
                                                  .mutex =
                                                    PTHREAD_MUTEX_INITIALIZER,
                                                  .task_queues = NULL,
                                                  .size = 0,
                                                  .capacity = 0};

em_proxying_queue* emscripten_proxy_get_system_queue(void) {
  return &system_proxying_queue;
}

// The head of the zombie list. Its mutex protects access to the list and its
// other fields are not used..
static em_proxying_queue zombie_list_head = {.zombie_prev = &zombie_list_head,
                                             .zombie_next = &zombie_list_head,
                                             .mutex =
                                               PTHREAD_MUTEX_INITIALIZER};

static void em_proxying_queue_free(em_proxying_queue* q) {
  pthread_mutex_destroy(&q->mutex);
  for (int i = 0; i < q->size; i++) {
    task_queue_deinit(&q->task_queues[i]);
  }
  free(q->task_queues);
  free(q);
}

static void cull_zombies() {
  pthread_mutex_lock(&zombie_list_head.mutex);
  em_proxying_queue* curr = zombie_list_head.zombie_next;
  while (curr != &zombie_list_head) {
    em_proxying_queue* next = curr->zombie_next;
    if (curr->js_refcount == 0) {
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
  *q = (em_proxying_queue){.js_refcount = 0,
                           .zombie_prev = NULL,
                           .zombie_next = NULL,
                           .mutex = PTHREAD_MUTEX_INITIALIZER,
                           .task_queues = NULL,
                           .size = 0,
                           .capacity = 0};
  return q;
}

void em_proxying_queue_destroy(em_proxying_queue* q) {
  assert(q != NULL);
  assert(q != &system_proxying_queue && "cannot destroy system proxying queue");
  assert(!q->zombie_next && !q->zombie_prev &&
         "double freeing em_proxying_queue!");
  if (q->js_refcount == 0) {
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
  int tasks_index = get_tasks_index_for_thread(q, pthread_self());
  task_queue* tasks = tasks_index == -1 ? NULL : &q->task_queues[tasks_index];
  if (tasks == NULL || tasks->processing) {
    // No tasks for this thread or they are already being processed.
    goto end;
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

end:
  pthread_mutex_unlock(&q->mutex);
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
    q->js_refcount++;
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
