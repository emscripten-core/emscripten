// In threaded builds dlopen copies the side module's file into linear memory
// so that other threads can load it.  Check that the copy is freed once every
// running thread has loaded the module, and that threads started afterwards can
// still load it, both from the main thread and from another thread, on workers
// that were created before the dlopen.

#include <assert.h>
#include <dlfcn.h>
#include <malloc.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

typedef int (*side_fn_t)(int);
static side_fn_t side_fn;
static _Atomic int go;

static void* nested_thread(void* arg) {
  return (void*)(long)side_fn(3);
}

// Running while the main thread calls dlopen.  Once released, it calls into the
// side module and starts another thread that does too.
static void* running_thread(void* arg) {
  while (!go) {
    usleep(1000);
  }
  pthread_t nested;
  void* result;
  pthread_create(&nested, NULL, nested_thread, NULL);
  pthread_join(nested, &result);
  assert((long)result == 4);
  return (void*)(long)side_fn(1);
}

// Started by the main thread after the dlopen.
static void* later_thread(void* arg) {
  return (void*)(long)side_fn(2);
}

int main() {
  pthread_t running, later;
  void* result;
  pthread_create(&running, NULL, running_thread, NULL);

  size_t used = mallinfo().uordblks;
  void* h = dlopen("/lib/libside.so", RTLD_NOW);
  assert(h);
  size_t growth = mallinfo().uordblks - used;
  printf("heap growth: %zuKB\n", growth >> 10);
  assert(growth < (1 << 20));
  // Threads that load the module from now on can't fall back to the file.
  unlink("/lib/libside.so");

  side_fn = (side_fn_t)dlsym(h, "side_fn");
  assert(side_fn && side_fn(0) == 1);

  // Start this while the first thread is still running, so that it runs on a
  // pool worker that hasn't loaded the module.
  pthread_create(&later, NULL, later_thread, NULL);
  pthread_join(later, &result);
  assert((long)result == 3);

  go = 1;
  pthread_join(running, &result);
  assert((long)result == 2);
  printf("done\n");
  return 0;
}
