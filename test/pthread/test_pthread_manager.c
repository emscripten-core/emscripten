#include <pthread.h>
#include <stdio.h>

#include <emscripten/em_js.h>

EM_JS(int, get_worker_id, (), {
  return workerID;
});


void *thread_main(void* arg) {
  printf("thread_main (workerID=%d)\n", get_worker_id());
  return NULL;
}

int main() {
  printf("main (workerID=%d)\n", get_worker_id());
  pthread_t t;

  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);

  // The second pthread should run on the same worker.
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);

  printf("done\n");
  return 0;
}
