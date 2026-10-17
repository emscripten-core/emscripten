// Proxied main must exit with its status after keepalives run.
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <stdio.h>
#include <stdlib.h>

void at_exit(void) { printf("done\n"); }

#ifdef MODE_CLEARED
int long_id;

void never(void* arg) {
  printf("cleared callback ran\n");
  abort();
}
#endif

void fired(void* arg) {
  printf("fired\n");
#ifdef MODE_CLEARED
  emscripten_clear_timeout(long_id);
#elif defined(MODE_FORCE_EXIT)
  // exit() beats a waiting return.
  emscripten_force_exit(7);
#elif defined(MODE_EXIT)
  exit(7);
#endif
}

int main(void) {
#ifdef MODE_NEGATIVE
  // Check the value, not just the code.
  MAIN_THREAD_EM_ASM({ Module['onExit'] = (c) => { out('exited:' + c); }; });
#else
  MAIN_THREAD_EM_ASM({ Module['onExit'] = () => { out('exited'); }; });
#endif
  atexit(at_exit);
#ifdef MODE_CLEARED
  long_id = emscripten_set_timeout(never, 10000, NULL);
#endif
  emscripten_set_timeout(fired, 10, NULL);
#ifdef MODE_NEGATIVE
  return -1;
#else
  return 3;
#endif
}
