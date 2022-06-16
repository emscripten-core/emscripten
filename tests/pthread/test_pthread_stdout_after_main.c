#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <emscripten.h>

void* thread_main(void* arg) {
  for (int i = 0; i < 10; i++) {
    printf("looping...\n");
    usleep(100*1000);
  }
  emscripten_force_exit(0);
  return NULL;
}

int main() {
  pthread_t t;
  int rc = pthread_create(&t, NULL, thread_main, NULL);
  assert(rc == 0);
  // Program is built without EXIT_RUNTIME so returning here
  // does not stop the program from running.
  return 0;
}
