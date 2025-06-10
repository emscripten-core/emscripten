/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>

static inline const char *emscripten_event_type_to_string(int eventType) {
  const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
    "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
    "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
    "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "mouseenter", "mouseleave", "mouseover", "mouseout", "(invalid)" };
  ++eventType;
  if (eventType < 0) eventType = 0;
  if (eventType >= sizeof(events)/sizeof(events[0])) eventType = sizeof(events)/sizeof(events[0])-1;
  return events[eventType];
}

const char *emscripten_result_to_string(EMSCRIPTEN_RESULT result) {
  if (result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
  if (result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
  if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
  if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
  if (result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
  if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
  if (result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
  if (result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
  if (result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
  return "Unknown EMSCRIPTEN_RESULT!";
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

bool gamepad_callback(int eventType, const EmscriptenGamepadEvent *e, void *userData) {
  printf("%s: timeStamp: %g, connected: %d, index: %d, numAxes: %d, numButtons: %d, id: \"%s\", mapping: \"%s\"\n",
    eventType != 0 ? emscripten_event_type_to_string(eventType) : "Gamepad state", e->timestamp, e->connected, e->index,
    e->numAxes, e->numButtons, e->id, e->mapping);

  if (e->connected) {
    for (int i = 0; i < e->numAxes; ++i)
      printf("Axis %d: %g\n", i, e->axis[i]);

    for (int i = 0; i < e->numButtons; ++i)
      printf("Button %d: Digital: %d, Analog: %g\n", i, e->digitalButton[i], e->analogButton[i]);
  }

  return 0;
}

EmscriptenGamepadEvent prevState[32];
int prevNumGamepads = 0;

void mainloop() {
  EMSCRIPTEN_RESULT res = emscripten_sample_gamepad_data();
  if (res != EMSCRIPTEN_RESULT_SUCCESS) {
    printf("emscripten_sample_gamepad_data returned EMSCRIPTEN_RESULT_NOT_SUPPORTED.\n");
    emscripten_cancel_main_loop();
    return;
  }

  int numGamepads = emscripten_get_num_gamepads();
  if (numGamepads != prevNumGamepads) {
    printf("Number of connected gamepads: %d\n", numGamepads);
    prevNumGamepads = numGamepads;
  }

  for (int i = 0; i < numGamepads && i < 32; ++i) {
    EmscriptenGamepadEvent ge;
    int ret = emscripten_get_gamepad_status(i, &ge);
    if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
      int g = ge.index;
      for (int j = 0; j < ge.numAxes; ++j) {
        if (ge.axis[j] != prevState[g].axis[j])
          printf("Gamepad %d, axis %d: %g\n", g, j, ge.axis[j]);
      }

      for (int j = 0; j < ge.numButtons; ++j) {
        if (ge.analogButton[j] != prevState[g].analogButton[j] || ge.digitalButton[j] != prevState[g].digitalButton[j])
          printf("Gamepad %d, button %d: Digital: %d, Analog: %g\n", g, j, ge.digitalButton[j], ge.analogButton[j]);
      }
      prevState[g] = ge;
    }
  }
}

void report_result(void *arg) {
  emscripten_html5_remove_all_event_listeners();
  emscripten_force_exit(0);
}

int main() {
  EMSCRIPTEN_RESULT ret = emscripten_set_gamepadconnected_callback(0, 1, gamepad_callback);
  TEST_RESULT(emscripten_set_gamepadconnected_callback);
  ret = emscripten_set_gamepaddisconnected_callback(0, 1, gamepad_callback);
  TEST_RESULT(emscripten_set_gamepaddisconnected_callback);

  emscripten_set_main_loop(mainloop, 10, 0);

  // Keep the page running for a moment.
  emscripten_async_call(report_result, 0, 5000);
  return 99;
}
