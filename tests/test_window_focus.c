#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

int result = 0;

EM_BOOL focusevent_callback(int eventType, const EmscriptenFocusEvent *e, void *userData) {
  result += 1;
  return 0;
}

void main_2(void* arg);

int main() {

  emscripten_async_call(main_2, NULL, 3000); // avoid startup delays and intermittent errors

  return 0;
}

void main_2(void* arg) {
  emscripten_set_blur_callback("#window", 0, 0, focusevent_callback);
  emscripten_run_script("var event = document.createEvent('Event'); event.initEvent('blur', true, false); window.dispatchEvent(event)");

  assert(result == 1);

  REPORT_RESULT();
}

