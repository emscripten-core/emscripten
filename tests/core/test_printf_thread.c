#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../system/lib/libc/musl/src/include/features.h"
#include "../../system/lib/libc/musl/src/internal/stdio_impl.h"
#include "../../system/lib/libc/musl/src/internal/libc.h"

pthread_t thread[2];

char *char_repeat(int n, char c) {
  char *dest = malloc(n + 1);
  memset(dest, c, n);
  dest[n] = '\0';
  return dest;
}

void *thread_main(void *arg) {
  char *msg = char_repeat(100, 'a');
  for (int i = 0; i < 10; ++i)
    printf("%s\n", msg);
  free(msg);
  return 0;
}

int main() {
  printf("stdin lock: %d\n", stdin->lock);
  printf("stdout lock: %d\n", stdout->lock);
  printf("stderr lock: %d\n", stderr->lock);
  printf("threaded libc: %d\n", libc.threaded);
  int rc = pthread_create(&thread[0], NULL, thread_main, NULL);
  assert(rc == 0);
  rc = pthread_create(&thread[1], NULL, thread_main, NULL);
  assert(rc == 0);
  void *thread_rtn = 0;
  rc = pthread_join(thread[0], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);
  rc = pthread_join(thread[1], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);

  return 0;
}
