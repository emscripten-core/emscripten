#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

typedef int (*func_t)();

func_t g_one;
func_t g_two;
func_t g_three;

void* open_lib() {
  void* handle = dlopen("liblib.so", RTLD_NOW|RTLD_GLOBAL);
  if (!handle) {
    printf("dlerror: %s\n", dlerror());
    assert(handle);
  }
  printf("dlopen returned: %p\n", handle);
  return handle;
}

static void test_order1() {
  void* handle = open_lib();

  g_one = dlsym(handle, "one");
  g_two = dlsym(handle, "two");
  g_three = dlsym(handle, "three");

  assert(g_one() == 1);
  assert(g_two() == 2);
  assert(g_three() == 3);
}

static void test_order2() {
  void* handle = open_lib();

  func_t three = dlsym(handle, "three");
  func_t two = dlsym(handle, "two");
  func_t one = dlsym(handle, "one");

  printf("one:   %p -> %d\n", one, one());
  printf("two:   %p -> %d\n", two, two());
  printf("three: %p -> %d\n", three, three());
  assert(one() == 1);
  assert(two() == 2);
  assert(three() == 3);

  assert(one == g_one);
  assert(two == g_two);
  assert(three == g_three);
}

static void* thread_main(void* arg) {
  printf("in thread_main\n");
  test_order2();
  printf("thread_main done\n");
  return 0;
}

int main() {
  printf("in main\n");
  test_order1();

  pthread_t t;
  int rc = pthread_create(&t, NULL, thread_main, NULL);
  assert(rc == 0);
  pthread_join(t, NULL);

  return 0;
}
