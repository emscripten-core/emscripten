#include <pthread.h>
#include <unistd.h>
#include <emscripten/threading.h>

void* worker_thread(void*) {
  emscripten_set_thread_name(pthread_self(), "test worker");
  for (int i = 0; i < 2; i++) {
    usleep(1000*1000);
  }
  return NULL;
}

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, worker_thread, NULL);
  pthread_join(thread, NULL);
  return 0;
}
