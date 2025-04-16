// Tests calling pthread_exit() from main() which should cause
// the program to exit with status 0.

#include <pthread.h>
#include <stdio.h>

int main() {
  printf("main\n");
  pthread_exit((void*)42);
  __builtin_trap();
  return 99;
}
