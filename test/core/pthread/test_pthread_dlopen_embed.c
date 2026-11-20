#include <assert.h>
#include <dlfcn.h>
#include <emscripten/emscripten.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef void (*func_t)();

static atomic_bool thread_done = false;
static pthread_t worker;

static void check_embedded_file(const char* who) {
  FILE* f = fopen("embedded.txt", "r");
  assert(f);
  char buf[64] = {0};
  size_t n = fread(buf, 1, sizeof(buf) - 1, f);
  assert(n > 0);
  assert(strcmp(buf, "hello from embedded file\n") == 0);
  fclose(f);
  printf("%s: %s", who, buf);
}

#ifdef USE_ASYNC_DLOPEN
static void on_dl_fail(void* userdata) {
  assert(0 && "emscripten_dlopen failed");
}

static void on_dl_success(void* userdata, void* handle) {
  assert(handle);
  func_t read_embedded_file = (func_t)dlsym(handle, "read_embedded_file");
  assert(read_embedded_file);
  read_embedded_file();
  check_embedded_file("pthread");
  dlclose(handle);
  thread_done = true;
}
#endif

static void* thread_main(void* arg) {
#ifdef USE_ASYNC_DLOPEN
  emscripten_dlopen("libside.so", RTLD_NOW | RTLD_GLOBAL, NULL, on_dl_success, on_dl_fail);
#else
  void* handle = dlopen("libside.so", RTLD_NOW | RTLD_GLOBAL);
  assert(handle);
  func_t read_embedded_file = (func_t)dlsym(handle, "read_embedded_file");
  assert(read_embedded_file);
  read_embedded_file();
  check_embedded_file("pthread");
  dlclose(handle);
  thread_done = true;
#endif
  return NULL;
}

#ifdef USE_ASYNC_DLOPEN
static void tick() {
  if (thread_done) {
    pthread_join(worker, NULL);
    check_embedded_file("main");
    emscripten_cancel_main_loop();
    printf("done\n");
  }
}
#endif

int main() {
  int rc = pthread_create(&worker, NULL, thread_main, NULL);
  assert(rc == 0);
#ifdef USE_ASYNC_DLOPEN
  emscripten_set_main_loop(tick, 0, 1);
#else
  pthread_join(worker, NULL);
  check_embedded_file("main");
  printf("done\n");
#endif
  return 0;
}
