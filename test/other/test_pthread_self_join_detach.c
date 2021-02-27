#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/threading.h>
#endif

int main() {
  /*
   * When running in PROXY_TO_PTHREAD mode the main thread
   * is already detached
   */
#ifdef __EMSCRIPTEN__
  int is_detached = !emscripten_is_main_browser_thread();
#else
  int is_detached = 0;
#endif
  pthread_t self = pthread_self();

  /*
   * Attempts to join the current thread will generate EDEADLK.
   */
  int ret = pthread_join(self, NULL);
  printf("pthread_join -> %s\n", strerror(ret));
  assert(ret == EDEADLK);

  /*
   * Attempts to detach the main thread will either succeed
   * or cause EINVAL if its already been detached.
   */
  ret = pthread_detach(self);
  printf("pthread_detach(self) -> %s\n", strerror(ret));
  if (is_detached) {
    assert(ret == EINVAL);
  } else {
    assert(ret == 0);
  }

  puts("passed");

  return 0;
}
