#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* thread_func(void* arg) {
  int* fd = (int*)arg;

  printf("thread main\n");
  int rc = close(*fd);
  printf("close: %d\n", rc);

  return NULL;
}

int main() {
  printf("main\n");
  int fd = open("example.txt", O_RDWR | O_CREAT, 0644);

  pthread_t t;
  int rc = pthread_create(&t, NULL, thread_func, &fd);
  printf("pthread_create: %d\n", rc);
  rc = pthread_join(t, NULL);
  printf("pthread_join: %d\n", rc);

  printf("done\n");

  return 0;
}
