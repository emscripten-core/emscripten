#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

int main() {
  printf("main\n");
  pthread_kill(pthread_self(), SIGTERM);
  assert(false && "should not get here");
}
