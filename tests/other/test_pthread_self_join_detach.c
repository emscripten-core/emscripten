#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include <emscripten/threading.h>

int main() {
  pthread_t self = pthread_self();

  /* Join the calling thread. This should fail with EDEADLK. */
  assert(pthread_join(self, NULL) == EDEADLK);

  /* We should be able to detach the main thread,
   * but not the proxied main thread (as that one
   * is already detached).
   */
  int ret = emscripten_is_main_browser_thread() ? 0 : EINVAL;
  assert(pthread_detach(self) == ret);

  puts("passed");

  return 0;
}
