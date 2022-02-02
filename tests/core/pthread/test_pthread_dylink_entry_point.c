#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>

typedef void* (*thread_main_t)(void*);

int main() {
  puts("hello from main");

  void *lib_handle = dlopen("./liblib.so", RTLD_NOW);
  if (!lib_handle) {
    puts("cannot load side module");
    puts(dlerror());
    return 1;
  }

  thread_main_t tmain = (thread_main_t)dlsym(lib_handle, "side_module_thread_main");
  if (!tmain) {
    puts("cannot find side function");
    return 1;
  }

  printf("starting thread with %p\n", tmain);
  pthread_t t;
  pthread_create(&t, NULL, tmain, (void*)"hello");
  pthread_join(t, NULL);

  printf("done\n");
  return 0;
}
