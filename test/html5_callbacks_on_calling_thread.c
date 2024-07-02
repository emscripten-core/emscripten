#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

// #define TEST_SYNC_BLOCKING_LOOP

void *mainRuntimeThreadId = 0;
void *registeringThreadId = 0;

bool mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  static int once;

  void *threadId = pthread_self();
  if (!once) {
    printf("pthread_self()=%p, registeringThreadId=%p, mainRuntimeThreadId=%p, "
           "emscripten_main_runtime_thread_id()=%p\n",
           threadId,
           registeringThreadId,
           mainRuntimeThreadId,
           emscripten_main_runtime_thread_id());
  }
  printf("eventType: %d, mouseEvent: %p, userData: %p, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, movement: (%ld,%ld), canvas: (%ld,%ld)\n",
    eventType, e, userData,
    e->screenX, e->screenY, e->clientX, e->clientY,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
    e->button, e->buttons, e->movementX, e->movementY, e->canvasX, e->canvasY);
  assert(threadId && threadId == registeringThreadId);
  assert(userData == (void*)0x42);
  assert(e);
  assert(eventType == EMSCRIPTEN_EVENT_MOUSEMOVE);

  if (once) {
    return 0;
  }
  once = 1;

#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
  return 0;
}

void *threadMain(void *arg) {
  registeringThreadId = pthread_self();

  EMSCRIPTEN_RESULT ret = emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)0x42, 1, mouse_callback);
  assert(ret == EMSCRIPTEN_RESULT_SUCCESS);

  printf("Please move the mouse cursor.\n");

#ifdef TEST_SYNC_BLOCKING_LOOP
  for (;;) {
    usleep(1000);
    emscripten_current_thread_process_queued_calls();
  }
#else
  emscripten_exit_with_live_runtime();
#endif
  return 0;
}

int main() {
  mainRuntimeThreadId = pthread_self();

  pthread_t thread;
  int rc = pthread_create(&thread, NULL, threadMain, 0);
  assert(rc == 0);

  emscripten_exit_with_live_runtime();
}
