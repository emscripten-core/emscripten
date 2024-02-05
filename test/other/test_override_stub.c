#include <stdio.h>
#include <unistd.h>
#include <emscripten.h>

pid_t fork() {
  printf("override called\n");
  return 1;
}

int main() {
  printf("%d\n", fork()); // re-implemented call
  chroot(NULL); // stubbed call
  return 0;
}
