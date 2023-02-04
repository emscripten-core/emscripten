/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/threading.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "em_task_queue.h"
#include "proxying_notification_state.h"

#define EM_TASK_QUEUE_INITIAL_CAPACITY 128

em_task_queue* em_task_queue_create(pthread_t thread) {
  em_task_queue* queue = malloc(sizeof(em_task_queue));
  if (queue == NULL) {
    return NULL;
  }
  task* tasks = malloc(sizeof(task) * EM_TASK_QUEUE_INITIAL_CAPACITY);
  if (tasks == NULL) {
    free(queue);
    return NULL;
  }
  *queue = (em_task_queue){.notification = NOTIFICATION_NONE,
                           .mutex = PTHREAD_MUTEX_INITIALIZER,
                           .thread = thread,
                           .processing = 0,
                           .tasks = tasks,
                           .capacity = EM_TASK_QUEUE_INITIAL_CAPACITY,
                           .head = 0,
                           .tail = 0};
  return queue;
}

void em_task_queue_destroy(em_task_queue* queue) {
  pthread_mutex_destroy(&queue->mutex);
  free(queue->tasks);
  free(queue);
}

// Not thread safe. Returns 1 on success and 0 on failure.
static int em_task_queue_grow(em_task_queue* queue) {
  // Allocate a larger task queue.
  int new_capacity = queue->capacity * 2;
  task* new_tasks = malloc(sizeof(task) * new_capacity);
  if (new_tasks == NULL) {
    return 0;
  }
  // Copy the tasks such that the head of the queue is at the beginning of the
  // buffer. There are two cases to handle: either the queue wraps around the
  // end of the old buffer or it does not.
  int queued_tasks;
  if (queue->head <= queue->tail) {
    // No wrap. Copy the tasks in one chunk.
    queued_tasks = queue->tail - queue->head;
    memcpy(new_tasks, &queue->tasks[queue->head], sizeof(task) * queued_tasks);
  } else {
    // Wrap. Copy `first_queued` tasks up to the end of the old buffer and
    // `last_queued` tasks at the beginning of the old buffer.
    int first_queued = queue->capacity - queue->head;
    int last_queued = queue->tail;
    queued_tasks = first_queued + last_queued;
    memcpy(new_tasks, &queue->tasks[queue->head], sizeof(task) * first_queued);
    memcpy(new_tasks + first_queued, queue->tasks, sizeof(task) * last_queued);
  }
  free(queue->tasks);
  queue->tasks = new_tasks;
  queue->capacity = new_capacity;
  queue->head = 0;
  queue->tail = queued_tasks;
  return 1;
}

void em_task_queue_execute(em_task_queue* queue) {
  queue->processing = 1;
  pthread_mutex_lock(&queue->mutex);
  while (!em_task_queue_is_empty(queue)) {
    task t = em_task_queue_dequeue(queue);
    // Unlock while the task is running to allow more work to be queued in
    // parallel.
    pthread_mutex_unlock(&queue->mutex);
    t.func(t.arg);
    pthread_mutex_lock(&queue->mutex);
  }
  pthread_mutex_unlock(&queue->mutex);
  queue->processing = 0;
}

int em_task_queue_enqueue(em_task_queue* queue, task t) {
  if (em_task_queue_is_full(queue) && !em_task_queue_grow(queue)) {
    return 0;
  }
  queue->tasks[queue->tail] = t;
  queue->tail = (queue->tail + 1) % queue->capacity;
  return 1;
}

task em_task_queue_dequeue(em_task_queue* queue) {
  task t = queue->tasks[queue->head];
  queue->head = (queue->head + 1) % queue->capacity;
  return t;
}

// Send a postMessage notification containing the em_task_queue pointer to the
// target thread so it will execute the queue when it returns to the event loop.
// Also pass in the current thread and main thread ids to minimize calls back
// into Wasm.
void _emscripten_notify_task_queue(pthread_t target_thread,
                                   pthread_t curr_thread,
                                   pthread_t main_thread,
                                   em_task_queue* queue);

void em_task_queue_notify(em_task_queue* queue) {
  // If there is no pending notification for this queue, create one. If an old
  // notification is currently being processed, it may or may not execute this
  // work. In case it does not, the new notification will ensure the work is
  // still executed.
  notification_state previous =
    atomic_exchange(&queue->notification, NOTIFICATION_PENDING);
  if (previous != NOTIFICATION_PENDING) {
    _emscripten_notify_task_queue(queue->thread,
                                  pthread_self(),
                                  emscripten_main_browser_thread_id(),
                                  queue);
  }
}
