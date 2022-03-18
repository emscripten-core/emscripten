#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <pthread.h>
#include <unistd.h>

#include "proxying.h"

em_proxying_queue* queue;

_Atomic int should_execute = 0;
_Atomic int executed = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void task(void* arg) {
  executed = 1;
  pthread_cond_signal(&cond);
}

void* execute_and_free_queue(void* arg) {
  // Wait until we are signaled to execute the queue.
  pthread_mutex_lock(&mutex);
  while (!should_execute) {
    pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  // Execute the proxied work then free the empty queue.
  emscripten_proxy_execute_queue(queue);
  em_proxying_queue_destroy(queue);

  // Exit with a live runtime so the queued work notification is received and we
  // try to execute the queue again, even though it has been destroyed.
  emscripten_exit_with_live_runtime();
}

int main() {
  emscripten_console_log("start");
  queue = em_proxying_queue_create();
  assert(queue);

  // Create the worker and send it a task.
  pthread_t worker;
  pthread_create(&worker, NULL, execute_and_free_queue, NULL);
  emscripten_proxy_async(queue, worker, task, NULL);
  should_execute = 1;
  pthread_cond_signal(&cond);

  // Wait for the task to be executed.
  pthread_mutex_lock(&mutex);
  while (!executed) {
    pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  // Wait a bit (50 ms) for the postmessage notification to be recieved and
  // processed.
  struct timespec time = {
    .tv_sec = 0,
    .tv_nsec = 50 * 1000 * 1000,
  };
  nanosleep(&time, NULL);

  // Cull the zombie!
  em_proxying_queue_destroy(em_proxying_queue_create());

  emscripten_console_log("done");
}
