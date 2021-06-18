#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "emscripten.h"
#include "emscripten/threading.h"

static _Atomic int threadCounter = 0;
static _Atomic int running = 1;
static pthread_t thread;

void *workerThread(void* arg) {
  threadCounter++;

  while (running)
    emscripten_thread_sleep(1000);

  threadCounter--;

  return NULL;
}

void terminateThread() {
  running = 0;

  int res = 0;
  int rc = pthread_join(thread, (void**)&res);
  assert(rc == 0);
  assert(res == 0);

  printf("done waiting - counter is: %d\n", threadCounter);
}

int main(int argc, char* argv[]) {
  int rc = atexit(terminateThread);
  assert(rc == 0);

  rc = pthread_create(&thread, NULL, workerThread, NULL);
  assert(rc == 0);
  return 0;
}
