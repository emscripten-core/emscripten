/*
 * Regression test for stdio locking.  If file locking is not enabled the
 * threads will race to write the file output buffer and we will see lines
 * that are longer or shorter then 100 characters.  When locking is
 * working/enabled each 100 character line will be printed separately.
 *
 * See:
 *   musl/src/stdio/__lockfile.c
 *   musl/src/stdio/fwrite.c
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
  printf("in main\n");
  void *thread_rtn;
  int rc;

  rc = pthread_create(&thread[0], NULL, thread_main, NULL);
  assert(rc == 0);

  rc = pthread_create(&thread[1], NULL, thread_main, NULL);
  assert(rc == 0);

  rc = pthread_join(thread[0], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);

  rc = pthread_join(thread[1], &thread_rtn);
  assert(rc == 0);
  assert(thread_rtn == 0);

  printf("main done\n");
  return 0;
}
