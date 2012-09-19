#include <stdio.h>
#include <gc.h>
#include <assert.h>
#include <emscripten.h>

void *global;

int freed = 0;

void finalizer(void *ptr, void *arg) {
  printf("finalizing %d (global == %d)\n", (int)arg, ptr == global);
  freed++;
  if (ptr == global) global = 0;
}

int stage = 0;
float start = 0;

void waiter(void*) {
  if (stage == 0) { // wait for a while, see no GCing
    assert(global);
    if (emscripten_get_now() - start > 2100) {
      GC_MALLOC(1024*1024*2); // allocate enough to trigger a GC
      start = emscripten_get_now();
      stage = 1;
      printf("stage 1\n");
    }
  } else if (stage == 1) {
    assert(global);
    if (freed > 0) {
      GC_FREE(global);
      stage = 2;
      start = emscripten_get_now();
      printf("stage 2\n");
    }
    if (emscripten_get_now() - start > 2100) {
      printf("fail, too much time passed (a)\n");
      return;
    }
  } else if (stage == 2) {
    if (emscripten_get_now() - start > 2100) { // wait and see that no gc'ing happens yet
      GC_MALLOC(1024*1024*2); // allocate enough to trigger a GC
      stage = 3;
      start = emscripten_get_now();
      printf("stage 3\n");
    }
  } else if (stage == 3) {
    assert(!global);
    if (freed == 5) {
      printf("Ok.\n");
      int result = 1;
      REPORT_RESULT();
      return;
    }
    if (emscripten_get_now() - start > 2100) {
      printf("fail, too much time passed (b)\n");
      return;
    }
  }

  emscripten_async_call(waiter, NULL, 100);
}

int main() {
  start = emscripten_get_now();

  GC_INIT();

  void *local, *local2, *local3, *local4;

  global = GC_MALLOC(12);
  GC_REGISTER_FINALIZER_NO_ORDER(global, finalizer, 0, 0, 0);
  local = GC_MALLOC(12);
  GC_REGISTER_FINALIZER_NO_ORDER(local, finalizer, (void*)1, 0, 0);
  local2 = GC_MALLOC_ATOMIC(12);
  GC_REGISTER_FINALIZER_NO_ORDER(local2, finalizer, (void*)2, 0, 0);
  local3 = GC_MALLOC(12);
  GC_REGISTER_FINALIZER_NO_ORDER(local3, finalizer, (void*)3, 0, 0);
  local4 = GC_MALLOC(12);
  GC_REGISTER_FINALIZER_NO_ORDER(local4, finalizer, (void*)4, 0, 0);

  void **globalData = (void**)global;
  globalData[0] = local;
  globalData[1] = local2;

  void **localData = (void**)local;
  localData[0] = local3;

  void **local2Data = (void**)local2;
  local2Data[0] = local4; // actually ignored, because local2 is atomic, so 4 is freeable

  emscripten_async_call(waiter, NULL, 100);

  return 0;
}

