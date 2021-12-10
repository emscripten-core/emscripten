/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/html5.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define TASK_QUEUE_INITIAL_CAPACITY 128

// TODO: Update this to take a `em_proxying_queue` argument.
extern int _emscripten_notify_thread_queue(pthread_t target_thread,
                                           pthread_t main_thread);

typedef struct task {
  void (*func)(void*);
  void* arg;
} task;

// A task queue for a particular thread. Organized into a linked list of
// task_queues for different threads.
typedef struct task_queue {
  // The target thread for this task_queue.
  pthread_t thread;
  // Recursion guard.
  int processing;
  // Ring buffer of tasks of size `capacity`. New tasks are enqueued at
  // `tail` and dequeued at `head`.
  task* tasks;
  int capacity;
  int head;
  int tail;
} task_queue;

static int task_queue_empty(task_queue* tasks) {
  return tasks->head == tasks->tail;
}

static int task_queue_full(task_queue* tasks) {
  return tasks->head == (tasks->tail + 1) % tasks->capacity;
}

// Returns 1 on success and 0 on failure.
static int task_queue_enqueue(task_queue* tasks, task t) {
  if (task_queue_full(tasks)) {
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
      memcpy(
        new_tasks, &tasks->tasks[tasks->head], sizeof(task) * queued_tasks);
    } else {
      // Wrap. Copy `first_queued` tasks up to the end of the old buffer and
      // `last_queued` tasks at the beginning of the old buffer.
      int first_queued = tasks->capacity - tasks->head;
      int last_queued = tasks->tail;
      queued_tasks = first_queued + last_queued;
      memcpy(
        new_tasks, &tasks->tasks[tasks->head], sizeof(task) * first_queued);
      memcpy(
        new_tasks + first_queued, tasks->tasks, sizeof(task) * last_queued);
    }
    free(tasks->tasks);
    tasks->tasks = new_tasks;
    tasks->capacity = new_capacity;
    tasks->head = 0;
    tasks->tail = queued_tasks;
  }
  // Enqueue the task.
  tasks->tasks[tasks->tail] = t;
  tasks->tail = (tasks->tail + 1) % tasks->capacity;
  return 1;
}

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

static em_proxying_queue system_proxying_queue = {
  PTHREAD_MUTEX_INITIALIZER, NULL, 0, 0};

em_proxying_queue* emscripten_proxy_get_system_queue(void) {
  return &system_proxying_queue;
}

em_proxying_queue* em_proxying_queue_create(void) {
  em_proxying_queue* q = malloc(sizeof(em_proxying_queue));
  if (q == NULL) {
    return NULL;
  }
  pthread_mutex_init(&q->mutex, NULL);
  q->task_queues = NULL;
  q->size = 0;
  q->capacity = 0;
  return q;
}

void em_proxying_queue_destroy(em_proxying_queue* q) {
  if (q == NULL) {
    return;
  }
  assert(q != &system_proxying_queue && "cannot destroy system proxying queue");
  // No need to acquire the lock; no one should be racing with the destruction
  // of the queue.
  pthread_mutex_destroy(&q->mutex);
  for (int i = 0; i < q->size; i++) {
    free(q->task_queues[i].tasks);
  }
  free(q->task_queues);
  free(q);
}

// Not thread safe.
static task_queue* get_tasks_for_thread(em_proxying_queue* q,
                                        pthread_t thread) {
  assert(q != NULL);
  task_queue* tasks = NULL;
  for (int i = 0; i < q->size; i++) {
    if (pthread_equal(q->task_queues[i].thread, thread)) {
      tasks = &q->task_queues[i];
      break;
    }
  }
  return tasks;
}

// Not thread safe.
static task_queue* get_or_add_tasks_for_thread(em_proxying_queue* q,
                                               pthread_t thread) {
  task_queue* tasks = get_tasks_for_thread(q, thread);
  if (tasks != NULL) {
    return tasks;
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
  tasks = &q->task_queues[q->size];
  tasks->thread = thread;
  tasks->processing = 0;
  tasks->tasks = malloc(sizeof(task) * TASK_QUEUE_INITIAL_CAPACITY);
  if (tasks->tasks == NULL) {
    return NULL;
  }
  tasks->head = 0;
  tasks->tail = 0;
  q->size++;
  return tasks;
}

void emscripten_proxy_execute_queue(em_proxying_queue* q) {
  if (q == NULL) {
    return;
  }
  pthread_mutex_lock(&q->mutex);
  task_queue* tasks = get_tasks_for_thread(q, pthread_self());
  if (tasks == NULL || tasks->processing) {
    // No tasks for this thread or they are already being processed.
    pthread_mutex_unlock(&q->mutex);
    return;
  }
  // Found the task queue; process the tasks.
  tasks->processing = 1;
  while (!task_queue_empty(tasks)) {
    task t = task_queue_dequeue(tasks);
    // Unlock while the task is running to allow more work to be queued in
    // parallel.
    pthread_mutex_unlock(&q->mutex);
    t.func(t.arg);
    pthread_mutex_lock(&q->mutex);
  }
  tasks->processing = 0;
  pthread_mutex_unlock(&q->mutex);
}

static pthread_t normalize_thread(pthread_t thread) {
  if (pthread_equal(thread, EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD)) {
    return emscripten_main_browser_thread_id();
  }
  if (pthread_equal(thread, EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)) {
    return pthread_self();
  }
  return thread;
}

int emscripten_proxy_async(em_proxying_queue* q,
                           pthread_t target_thread,
                           void (*func)(void*),
                           void* arg) {
  if (q == NULL) {
    return 0;
  }
  target_thread = normalize_thread(target_thread);
  pthread_mutex_lock(&q->mutex);
  task_queue* tasks = get_or_add_tasks_for_thread(q, target_thread);
  if (tasks == NULL) {
    pthread_mutex_unlock(&q->mutex);
    return 0;
  }
  int empty = task_queue_empty(tasks);
  if (!task_queue_enqueue(tasks, (task){func, arg})) {
    pthread_mutex_unlock(&q->mutex);
    return 0;
  }
  pthread_mutex_unlock(&q->mutex);
  // If the queue was previously empty, notify the target thread to process it.
  // Otherwise, the target thread was already notified when the existing work
  // was enqueued so we don't need to notify it again.
  if (empty) {
    // TODO: Add `q` to this notification so the target thread knows which queue
    // to process.
    _emscripten_notify_thread_queue(target_thread,
                                    emscripten_main_browser_thread_id());
  }
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
  ctx->func = func;
  ctx->arg = arg;
  ctx->done = 0;
  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_cond_init(&ctx->cond, NULL);
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
  assert(!pthread_equal(normalize_thread(target_thread), pthread_self()) &&
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
