#include <emscripten/proxying.h>
#include <emscripten.h>
#include <stdio.h>

pthread_t main_thread, looper, returner, worker, worker2;
em_proxying_queue* proxy_queue = NULL;

void* dummy(void* arg) { return NULL; }
void dummy2(void* arg) {}

void* worker2_main(void* arg) {
  pthread_t self = pthread_self();
  const char* name = pthread_equal(self, worker2) ? "worker2" : "unknown";
  printf("pthread_self() == %s (%p)\n", name, (void*)self);
  return NULL;
}

int main(int argc, char* argv[]) {
  main_thread = pthread_self();
  proxy_queue = em_proxying_queue_create();
  pthread_create(&looper, NULL, dummy, NULL);
  pthread_create(&returner, NULL, dummy, NULL);
  emscripten_proxy_callback(proxy_queue, pthread_self(), dummy2, dummy2, NULL, NULL);
  pthread_create(&worker, NULL, dummy, 0);
  pthread_join(worker, NULL);
  pthread_create(&worker2, NULL, worker2_main, 0);
  pthread_join(worker2, NULL);
}
