#include <stdio.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

bool mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  printf("Mouse click on canvas.\n");
  emscripten_force_exit(0);
  return 0;
}

int main() {
  printf("The mouse cursor should be hidden when hovering over the canvas rectangle. Click on the canvas to finish test.\n");
  emscripten_hide_mouse();
  EMSCRIPTEN_RESULT ret = emscripten_set_click_callback("#canvas", 0, 1, mouse_callback);
  assert(ret == 0);
  emscripten_exit_with_live_runtime();
  return 0;
}
