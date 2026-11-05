// dlopen on the main runtime thread must sync all running threads at once, not
// one after another.  Two threads that don't yield wait for a thread that has
// already loaded the module: if the threads were synced in turn, whichever of
// them comes before that thread would block the sync forever.  Another thread
// exits without yielding, so the sync of that thread gets canceled.

#include <assert.h>
#include <dlfcn.h>
#include <emscripten/em_asm.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

_Atomic int loading;
_Atomic int loaded_on_thread;
_Atomic int done;

// Whether this thread's runtime has loaded the side module.
int side_module_loaded() {
  return EM_ASM_INT({
    return Object.keys(LDSO.loadedLibsByName).some((n) => n.includes('libside'));
  });
}

// Doesn't yield until another thread has loaded the module.
void* busy_thread(void* arg) {
  while (!loaded_on_thread) {
  }
  return NULL;
}

// Sleeps, which processes the sync, until the module is loaded here.
void* syncing_thread(void* arg) {
  while (!side_module_loaded()) {
    emscripten_thread_sleep(1);
  }
  loaded_on_thread = 1;
  return NULL;
}

// Exits without yielding once the dlopen has started.
void* exiting_thread(void* arg) {
  while (!loading) {
  }
  double end = emscripten_get_now() + 50;
  while (emscripten_get_now() < end) {
  }
  return NULL;
}

int main() {
  pthread_t threads[4];
  void* (*funcs[4])(void*) = {
    busy_thread, syncing_thread, busy_thread, exiting_thread};
  for (int i = 0; i < 4; i++) {
    int rc = pthread_create(&threads[i], NULL, funcs[i], NULL);
    assert(rc == 0);
  }
  emscripten_thread_sleep(50);

  loading = 1;
  void* h = dlopen("/lib/libside.so", RTLD_NOW);
  assert(h);
  int (*side_fn)(int) = (int (*)(int))dlsym(h, "side_fn");
  assert(side_fn && side_fn(1) == 2);

  for (int i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
  }
  printf("done\n");
  return 0;
}
