/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <pthread.h>

#include "proxying_notification_state.h"

// A task is an arbitrary function combined with some arbitrary state.
typedef struct task {
  void (*func)(void*);
  void* arg;
} task;

// A task queue holding tasks to be processed by a particular thread.
typedef struct em_task_queue {
  // Flag encoding the state of postMessage notifications for this task queue.
  // Accessed directly from JS, so must be the first member.
  _Atomic notification_state notification;
  // Protects all modifications to mutable `em_task_queue` state.
  pthread_mutex_t mutex;
  // The target thread for this em_task_queue. Immutable and accessible without
  // acquiring the mutex.
  pthread_t thread;
  // Recursion guard. Only accessed on the target thread, so there's no need to
  // hold the lock when accessing it. TODO: We disallow recursive processing
  // because that's what the old proxying API does, so it is safer to start with
  // the same behavior. Experiment with relaxing this restriction once the old
  // API uses these queues as well.
  int processing;
  // Ring buffer of tasks of size `capacity`. New tasks are enqueued at
  // `tail` and dequeued at `head`.
  task* tasks;
  int capacity;
  int head;
  int tail;
} em_task_queue;

em_task_queue* em_task_queue_create(pthread_t thread);

void em_task_queue_destroy(em_task_queue* queue);

// Execute tasks until an empty queue is observed.
void em_task_queue_execute(em_task_queue* queue);

// Not thread safe.
static inline int em_task_queue_is_empty(em_task_queue* queue) {
  return queue->head == queue->tail;
}

// Not thread safe.
static inline int em_task_queue_is_full(em_task_queue* queue) {
  return queue->head == (queue->tail + 1) % queue->capacity;
}

// Not thread safe. Returns 1 on success and 0 on failure.
int em_task_queue_enqueue(em_task_queue* queue, task t);

// Not thread safe. Assumes the queue is not empty.
task em_task_queue_dequeue(em_task_queue* queue);

// Schedule the queue to be executed next time its owning thread returns to its
// event loop.
void em_task_queue_notify(em_task_queue* queue);
