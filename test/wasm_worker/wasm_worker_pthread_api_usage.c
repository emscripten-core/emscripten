#include <pthread.h>

int main() {
  // This program should fail to link with WASM_WORKERS.
  pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&my_mutex);
}
