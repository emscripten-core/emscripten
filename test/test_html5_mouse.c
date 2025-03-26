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

void report_result(int result) {
  if (result == 0) {
    printf("Test successful!\n");
  } else {
    printf("Test failed!\n");
  }
#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}

static inline const char *emscripten_event_type_to_string(int eventType) {
  const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize", 
    "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange", 
    "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload", 
    "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)" };
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

int gotClick = 0;
int gotMouseDown = 0;
int gotMouseUp = 0;
int gotDblClick = 0;
int gotMouseMove = 0;
int gotWheel = 0;

void instruction() {
  if (!gotClick) { printf("Please click on the canvas.\n"); return; }
  if (!gotMouseDown) { printf("Please click on the canvas.\n"); return; }
  if (!gotMouseUp) { printf("Please click on the canvas.\n"); return; }
  if (!gotDblClick) { printf("Please double-click on the canvas.\n"); return; }
  if (!gotMouseMove) { printf("Please move the mouse on the canvas.\n"); return; }
  if (!gotWheel) { printf("Please scroll the mouse wheel.\n"); return; }

  if (gotClick && gotMouseDown && gotMouseUp && gotDblClick && gotMouseMove && gotWheel) report_result(0);
}

bool mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  printf("%s, screen: (%d,%d), client: (%d,%d),%s%s%s%s button: %hu, buttons: %hu, movement: (%d,%d), target: (%d, %d)\n",
    emscripten_event_type_to_string(eventType), e->screenX, e->screenY, e->clientX, e->clientY,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
    e->button, e->buttons, e->movementX, e->movementY, e->targetX, e->targetY);

  if (e->screenX != 0 && e->screenY != 0 && e->clientX != 0 && e->clientY != 0 && e->targetX != 0 && e->targetY != 0) {
    if (eventType == EMSCRIPTEN_EVENT_CLICK) gotClick = 1;
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN && e->buttons != 0) gotMouseDown = 1;
    if (eventType == EMSCRIPTEN_EVENT_DBLCLICK) gotDblClick = 1;
    if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) gotMouseUp = 1;
    if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && (e->movementX != 0 || e->movementY != 0)) gotMouseMove = 1;
  }

  if (eventType == EMSCRIPTEN_EVENT_CLICK && e->screenX == -500000) {
    printf("ERROR! Received an event to a callback that should have been unregistered!\n");
    gotClick = 0;
    report_result(1);
  }

  instruction();
  return 0;
}

bool wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData) {
  printf("%s, screen: (%d,%d), client: (%d,%d),%s%s%s%s button: %hu, buttons: %hu, target: (%d, %d), delta:(%g,%g,%g), deltaMode:%u\n",
    emscripten_event_type_to_string(eventType), e->mouse.screenX, e->mouse.screenY, e->mouse.clientX, e->mouse.clientY,
    e->mouse.ctrlKey ? " CTRL" : "", e->mouse.shiftKey ? " SHIFT" : "", e->mouse.altKey ? " ALT" : "", e->mouse.metaKey ? " META" : "", 
    e->mouse.button, e->mouse.buttons, e->mouse.targetX, e->mouse.targetY,
    (float)e->deltaX, (float)e->deltaY, (float)e->deltaZ, e->deltaMode);

  if (e->deltaY > 0.f || e->deltaY < 0.f)
    gotWheel = 1;

  instruction();
  return 0;
}

int main() {
  // Make the canvas area stand out from the background.
  emscripten_set_canvas_element_size("#canvas", 400, 300);
  EM_ASM(Module['canvas'].style.backgroundColor = 'black';);

  EMSCRIPTEN_RESULT ret = emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_click_callback);
  ret = emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mousedown_callback);
  ret = emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseup_callback);
  ret = emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_dblclick_callback);
  ret = emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mousemove_callback);

  ret = emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, wheel_callback);
  TEST_RESULT(emscripten_set_wheel_callback);

#ifdef AUTOMATE_SUCCESS
  EM_ASM(
    function sendEvent(type, data) {
      var event = document.createEvent('Event');
      event.initEvent(type, true, true);
      for (var d in data) event[d] = data[d];
      window.dispatchEvent(event);
    }
    sendEvent('click', { screenX: 123, screenY: 456, clientX: 123, clientY: 456, button: 0, buttons: 1 });
  );

  // get_mouse_status should contain the results of the last event
  EmscriptenMouseEvent mouseEvent;
  ret = emscripten_get_mouse_status(&mouseEvent);
  TEST_RESULT(emscripten_get_mouse_status);

  if (mouseEvent.screenX != 123 || mouseEvent.screenY != 456
    || mouseEvent.clientX != 123 || mouseEvent.clientY != 456) {
    printf("ERROR! Incorrect mouse status\n");
    report_result(1);
  }

  // Test that unregistering a callback works. Clicks should no longer be received.
  ret = emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, 0);
  TEST_RESULT(emscripten_set_click_callback);

  EM_ASM(
    function sendEvent(type, data) {
      var event = document.createEvent('Event');
      event.initEvent(type, true, true);
      for (var d in data) event[d] = data[d];
      window.dispatchEvent(event);
    }
    sendEvent('click', { screenX: -500000, screenY: -500000, clientX: -500000, clientY: -500000, button: 0, buttons: 0 }); // Send a dummy event that should not be received.
    sendEvent('mousedown', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 1 });
    sendEvent('mouseup', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0 });
    sendEvent('dblclick', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0 });
    sendEvent('mousemove', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0, 'movementX': 1, 'movementY': 1 });
    sendEvent('wheel', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0, 'deltaX': 1, 'deltaY': 1, 'deltaZ': 1, 'deltaMode': 1 });
    sendEvent('mousewheel', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0, 'wheelDeltaX': 1, 'wheelDeltaY': 1 });
  );
#endif

  return 0;
}
