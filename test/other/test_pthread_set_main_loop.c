#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <emscripten.h>

_Atomic bool term = false;
_Atomic bool loopthread_done = false;
pthread_t thread;
int loopcount = 0;

void loop() {
  printf("loop thread: %d\n", loopcount);

  if (loopcount++ > 10) {
    // This should exit the thread and allow join to complete below, but it doesn't work
    emscripten_cancel_main_loop();
    printf("loop thread done\n");
    loopthread_done = true;
  }
}

void* loopthread(void* arg) {
  emscripten_set_main_loop(loop, 0, false);
  return NULL;
}

void mainloop() {
  if (loopthread_done) {
    printf("joinng..\n");
    pthread_join(thread, NULL);
    printf("join done\n");
    emscripten_cancel_main_loop();
  }
}

int main() {
  pthread_create(&thread, NULL, loopthread, NULL);
  emscripten_set_main_loop(mainloop, 0, false);
  return 0;
}
