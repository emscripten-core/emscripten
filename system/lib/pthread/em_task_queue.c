/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/threading.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "em_task_queue.h"
#include "proxying_notification_state.h"
#include "thread_mailbox.h"

#define EM_TASK_QUEUE_INITIAL_CAPACITY 128

// Task Queue Lifetime Management
// -------------------------------
//
// When tasks are added to a task queue, the Worker running the target thread
// receives an event that will cause it to execute the queue when it next
// returns to its event loop. In some cases the queue will already have been
// executed before then, but the event is still received and the queue is still
// executed. These events contain references to the queue so that the target
// thread will know which queue to execute.
//
// To avoid use-after-free bugs, we cannot free a task queue immediately when
// `em_task_queue_destroy` is called; instead, we must defer freeing the queue
// until all of its outstanding notifications have been processed. We defer
// freeing the queue using an atomic flag. Each time a notification containing a
// reference to a task queue is generated, we set the flag on that task queue.
// Each time that task queue is processed, we clear the flag as long as another
// notification for the queue has not been generated in the mean time. The
// proxying queue can only be freed once `em_task_queue_destroy` has been called
// and its notification flag has been cleared.
//
// But an extra complication is that the target thread may have died by the time
// it gets back to its event loop to process its notifications. In that case the
// thread's Worker will still receive a notification and have to clear the
// notification flag without a live runtime. Without a live runtime, there is no
// stack, so the worker cannot safely free the queue at this point even if the
// notification flag is cleared. We need a separate thread with a live runtime
// to perform the free.
//
// To ensure that queues are eventually freed, we place destroyed queues in a
// global "zombie list" where they wait for their notification flags to be
// cleared. The zombie list is scanned and zombie queues without outstanding
// notifications are freed whenever a new queue is constructed. In principle the
// zombie list could be scanned at any time, but the queue constructor is a nice
// place to do it because scanning there is sufficient to keep the number of
// zombie queues from growing without bound; creating a new zombie ultimately
// requires creating a new queue.
//
// -------------------------------

// The head of the zombie list. Its mutex protects access to the list and its
// other fields are not used.
static em_task_queue zombie_list_head = {.mutex = PTHREAD_MUTEX_INITIALIZER,
                                         .zombie_prev = &zombie_list_head,
                                         .zombie_next = &zombie_list_head};

static void em_task_queue_free(em_task_queue* queue) {
  pthread_mutex_destroy(&queue->mutex);
  free(queue->tasks);
  free(queue);
}

static void cull_zombies() {
  if (pthread_mutex_trylock(&zombie_list_head.mutex) != 0) {
    // Some other thread is already culling. In principle there may be new
    // cullable zombies after it finishes, but it's not worth waiting to find
    // out.
    return;
  }
  em_task_queue* curr = zombie_list_head.zombie_next;
  while (curr != &zombie_list_head) {
    em_task_queue* next = curr->zombie_next;
    if (curr->notification == NOTIFICATION_NONE) {
      // Remove the zombie from the list and free it.
      curr->zombie_prev->zombie_next = curr->zombie_next;
      curr->zombie_next->zombie_prev = curr->zombie_prev;
      em_task_queue_free(curr);
    }
    curr = next;
  }
  pthread_mutex_unlock(&zombie_list_head.mutex);
}

em_task_queue* em_task_queue_create(pthread_t thread) {
  // Free any queue that has been destroyed and is safe to free.
  cull_zombies();

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
                           .tail = 0,
                           .zombie_prev = NULL,
                           .zombie_next = NULL};
  return queue;
}

void em_task_queue_destroy(em_task_queue* queue) {
  assert(queue->zombie_next == NULL && queue->zombie_prev == NULL);
  if (queue->notification == NOTIFICATION_NONE) {
    // No outstanding references to the queue, so we can go ahead and free it.
    em_task_queue_free(queue);
    return;
  }
  // Otherwise add the queue to the zombie list so that it will eventually be
  // freed safely.
  pthread_mutex_lock(&zombie_list_head.mutex);
  queue->zombie_next = &zombie_list_head;
  queue->zombie_prev = zombie_list_head.zombie_prev;
  queue->zombie_next->zombie_prev = queue;
  queue->zombie_prev->zombie_next = queue;
  pthread_mutex_unlock(&zombie_list_head.mutex);
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

void em_task_queue_cancel(em_task_queue* queue) {
  pthread_mutex_lock(&queue->mutex);
  while (!em_task_queue_is_empty(queue)) {
    task t = em_task_queue_dequeue(queue);
    if (t.cancel) {
      t.cancel(t.arg);
    }
  }
  pthread_mutex_unlock(&queue->mutex);
  // Any subsequent messages to this queue (for example if a pthread struct is
  // reused for a future thread, potentially on a different worker) will require
  // a new notification. Clearing the flag is safe here because in both the
  // proxying queue and mailbox cases, there are no more outstanding references
  // to the queue after thread shutdown.
  queue->notification = NOTIFICATION_NONE;
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

static void receive_notification(void* arg) {
  em_task_queue* tasks = arg;
  tasks->notification = NOTIFICATION_RECEIVED;
  em_task_queue_execute(tasks);
  notification_state expected = NOTIFICATION_RECEIVED;
  atomic_compare_exchange_strong(
    &tasks->notification, &expected, NOTIFICATION_NONE);
}

static void cancel_notification(void* arg) {
  em_task_queue* tasks = arg;
  em_task_queue_cancel(tasks);
}

int em_task_queue_send(em_task_queue* queue, task t) {
  // Ensure the target mailbox will remain open or detect that it is already
  // closed.
  if (!emscripten_thread_mailbox_ref(queue->thread)) {
    return 0;
  }

  pthread_mutex_lock(&queue->mutex);
  int enqueued = em_task_queue_enqueue(queue, t);
  pthread_mutex_unlock(&queue->mutex);
  if (!enqueued) {
    emscripten_thread_mailbox_unref(queue->thread);
    return 0;
  }

  // We're done if there is already a pending notification for this task queue.
  // Otherwise, we will send one.
  notification_state previous =
    atomic_exchange(&queue->notification, NOTIFICATION_PENDING);
  if (previous == NOTIFICATION_PENDING) {
    emscripten_thread_mailbox_unref(queue->thread);
    return 1;
  }

  emscripten_thread_mailbox_send(queue->thread,
                                 (task){.func = receive_notification,
                                        .cancel = cancel_notification,
                                        .arg = queue});
  emscripten_thread_mailbox_unref(queue->thread);
  return 1;
}
