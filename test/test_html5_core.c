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

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
bool key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  printf("%s, key: \"%s\", code: \"%s\", location: %u,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %u, keyCode: %u, which: %u, timestamp: %lf\n",
    emscripten_event_type_to_string(eventType), e->key, e->code, e->location,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
    e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which,
    e->timestamp);

  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "f") || e->which == 102)) {
    EmscriptenFullscreenChangeEvent fsce;
    EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);
    TEST_RESULT(emscripten_get_fullscreen_status);
    if (!fsce.isFullscreen) {
      printf("Requesting fullscreen..\n");
      ret = emscripten_request_fullscreen("#canvas", 1);
      TEST_RESULT(emscripten_request_fullscreen);
    } else {
      printf("Exiting fullscreen..\n");
      ret = emscripten_exit_fullscreen();
      TEST_RESULT(emscripten_exit_fullscreen);
      ret = emscripten_get_fullscreen_status(&fsce);
      TEST_RESULT(emscripten_get_fullscreen_status);
      if (fsce.isFullscreen) {
        fprintf(stderr, "Fullscreen exit did not work!\n");
      }
    }
  }

  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "p") || e->which == 112)) {
    EmscriptenPointerlockChangeEvent plce;
    EMSCRIPTEN_RESULT ret = emscripten_get_pointerlock_status(&plce);
    TEST_RESULT(emscripten_get_pointerlock_status);
    if (!plce.isActive) {
      printf("Requesting pointer lock..\n");
      ret = emscripten_request_pointerlock("#canvas", 1);
      TEST_RESULT(emscripten_request_pointerlock);
    } else {
      printf("Exiting pointer lock..\n");
      ret = emscripten_exit_pointerlock();
      TEST_RESULT(emscripten_exit_pointerlock);
      ret = emscripten_get_pointerlock_status(&plce);
      TEST_RESULT(emscripten_get_pointerlock_status);
      if (plce.isActive) {
        fprintf(stderr, "Pointer lock exit did not work!\n");
      }
    }
  }

  return 0;
}

bool mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  printf("%s, screen: (%d,%d), client: (%d,%d),%s%s%s%s button: %hu, buttons: %hu, movement: (%d,%d), canvas: (%d,%d), timestamp: %lf\n",
    emscripten_event_type_to_string(eventType), e->screenX, e->screenY, e->clientX, e->clientY,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
    e->button, e->buttons, e->movementX, e->movementY, e->canvasX, e->canvasY,
    e->timestamp);

  return 0;
}

bool wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData) {
  printf("%s, screen: (%d,%d), client: (%d,%d),%s%s%s%s button: %hu, buttons: %hu, canvas: (%d,%d), delta:(%g,%g,%g), deltaMode:%u, timestamp: %lf\n",
    emscripten_event_type_to_string(eventType), e->mouse.screenX, e->mouse.screenY, e->mouse.clientX, e->mouse.clientY,
    e->mouse.ctrlKey ? " CTRL" : "", e->mouse.shiftKey ? " SHIFT" : "", e->mouse.altKey ? " ALT" : "", e->mouse.metaKey ? " META" : "",
    e->mouse.button, e->mouse.buttons, e->mouse.canvasX, e->mouse.canvasY,
    (float)e->deltaX, (float)e->deltaY, (float)e->deltaZ, e->deltaMode,
    e->mouse.timestamp);

  return 0;
}

bool uievent_callback(int eventType, const EmscriptenUiEvent *e, void *userData) {
  printf("%s, detail: %d, document.body.client size: (%d,%d), window.inner size: (%d,%d), scrollPos: (%d, %d)\n",
    emscripten_event_type_to_string(eventType), e->detail, e->documentBodyClientWidth, e->documentBodyClientHeight,
    e->windowInnerWidth, e->windowInnerHeight, e->scrollTop, e->scrollLeft);

  return 0;
}

bool focusevent_callback(int eventType, const EmscriptenFocusEvent *e, void *userData) {
  printf("%s, nodeName: \"%s\", id: \"%s\"\n", emscripten_event_type_to_string(eventType), e->nodeName, e->id[0] == '\0' ? "(empty string)" : e->id);

  return 0;
}

bool deviceorientation_callback(int eventType, const EmscriptenDeviceOrientationEvent *e, void *userData) {
  printf("%s, (%g, %g, %g)\n", emscripten_event_type_to_string(eventType), e->alpha, e->beta, e->gamma);

  return 0;
}

bool devicemotion_callback(int eventType, const EmscriptenDeviceMotionEvent *e, void *userData) {
  printf("%s, accel: (%g, %g, %g), accelInclGravity: (%g, %g, %g), rotationRate: (%g, %g, %g), supportedFields: %s %s %s\n",
    emscripten_event_type_to_string(eventType),
    e->accelerationX, e->accelerationY, e->accelerationZ,
    e->accelerationIncludingGravityX, e->accelerationIncludingGravityY, e->accelerationIncludingGravityZ,
    e->rotationRateAlpha, e->rotationRateBeta, e->rotationRateGamma,
    (e->supportedFields & EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION) ? "EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION" : "",
    (e->supportedFields & EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION_INCLUDING_GRAVITY) ? "EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION_INCLUDING_GRAVITY" : "",
    (e->supportedFields & EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ROTATION_RATE) ? "EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ROTATION_RATE" : "");

  return 0;
}

bool orientationchange_callback(int eventType, const EmscriptenOrientationChangeEvent *e, void *userData) {
  printf("%s, orientationAngle: %d, orientationIndex: %d\n", emscripten_event_type_to_string(eventType), e->orientationAngle, e->orientationIndex);

  return 0;
}

bool fullscreenchange_callback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData) {
  printf("%s, isFullscreen: %d, fullscreenEnabled: %d, fs element nodeName: \"%s\", fs element id: \"%s\". New size: %dx%d pixels. Screen size: %dx%d pixels.\n",
    emscripten_event_type_to_string(eventType), e->isFullscreen, e->fullscreenEnabled, e->nodeName, e->id, e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);

  return 0;
}

bool pointerlockchange_callback(int eventType, const EmscriptenPointerlockChangeEvent *e, void *userData) {
  printf("%s, isActive: %d, pointerlock element nodeName: \"%s\", id: \"%s\"\n",
    emscripten_event_type_to_string(eventType), e->isActive, e->nodeName, e->id);

  return 0;
}

bool visibilitychange_callback(int eventType, const EmscriptenVisibilityChangeEvent *e, void *userData) {
  printf("%s, hidden: %d, visibilityState: %d\n", emscripten_event_type_to_string(eventType), e->hidden, e->visibilityState);

  return 0;
}

bool touch_callback(int eventType, const EmscriptenTouchEvent *e, void *userData) {
  printf("%s, numTouches: %d timestamp: %lf %s%s%s%s\n",
    emscripten_event_type_to_string(eventType), e->numTouches, e->timestamp,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "");
  for (int i = 0; i < e->numTouches; ++i) {
    const EmscriptenTouchPoint *t = &e->touches[i];
    printf("  %d: screen: (%d,%d), client: (%d,%d), page: (%d,%d), isChanged: %d, onTarget: %d, canvas: (%d, %d)\n",
      t->identifier, t->screenX, t->screenY, t->clientX, t->clientY, t->pageX, t->pageY, t->isChanged, t->onTarget, t->canvasX, t->canvasY);
  }

  return 0;
}

const char *beforeunload_callback(int eventType, const void *reserved, void *userData) {
#ifdef REPORT_RESULT
  return ""; // For test harness, don't show a confirmation dialog to not block and keep the test runner automated.
#else
  return "Do you really want to leave the page?";
#endif
}

void formatTime(char *str, int seconds) {
  int h = seconds / (60*60);
  seconds -= h*60*60;
  int m = seconds / 60;
  seconds -= m*60;
  if (h > 0)
  {
    sprintf(str, "%dh:%02dm:%02ds", h, m, seconds);
  }
  else
  {
    sprintf(str, "%02dm:%02ds", m, seconds);
  }
}

bool battery_callback(int eventType, const EmscriptenBatteryEvent *e, void *userData) {
  char t1[64];
  formatTime(t1, (int)e->chargingTime);
  char t2[64];
  formatTime(t2, (int)e->dischargingTime);
  printf("%s: chargingTime: %s, dischargingTime: %s, level: %g%%, charging: %d\n",
    emscripten_event_type_to_string(eventType), t1, t2, e->level*100, e->charging);

  return 0;
}

bool webglcontext_callback(int eventType, const void *reserved, void *userData) {
  printf("%s.\n", emscripten_event_type_to_string(eventType));

  return 0;
}

#ifndef KEEP_ALIVE
void test_done(void *arg) {
  emscripten_html5_remove_all_event_listeners();
  exit(0);
}
#endif

int main() {
  EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
  TEST_RESULT(emscripten_set_keypress_callback);
  ret = emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
  TEST_RESULT(emscripten_set_keydown_callback);
  ret = emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
  TEST_RESULT(emscripten_set_keyup_callback);

  ret = emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_click_callback);
  ret = emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mousedown_callback);
  ret = emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseup_callback);
  ret = emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_dblclick_callback);
  ret = emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mousemove_callback);
  ret = emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseenter_callback);
  ret = emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseleave_callback);
  ret = emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseover_callback);
  ret = emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseout_callback);

  ret = emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, wheel_callback);
  TEST_RESULT(emscripten_set_wheel_callback);

  ret = emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, uievent_callback);
  TEST_RESULT(emscripten_set_resize_callback);
  ret = emscripten_set_scroll_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, uievent_callback);
  TEST_RESULT(emscripten_set_scroll_callback);

  ret = emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
  TEST_RESULT(emscripten_set_blur_callback);
  ret = emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
  TEST_RESULT(emscripten_set_focus_callback);
  ret = emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
  TEST_RESULT(emscripten_set_focusin_callback);
  ret = emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
  TEST_RESULT(emscripten_set_focusout_callback);

  ret = emscripten_set_deviceorientation_callback(0, 1, deviceorientation_callback);
  TEST_RESULT(emscripten_set_deviceorientation_callback);
  ret = emscripten_set_devicemotion_callback(0, 1, devicemotion_callback);
  TEST_RESULT(emscripten_set_devicemotion_callback);

  ret = emscripten_set_orientationchange_callback(0, 1, orientationchange_callback);
  TEST_RESULT(emscripten_set_orientationchange_callback);

  // Test the polling of orientation.
  EmscriptenOrientationChangeEvent oce;
  ret = emscripten_get_orientation_status(&oce);
  TEST_RESULT(emscripten_get_orientation_status);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("The current orientation is:\n");
    orientationchange_callback(EMSCRIPTEN_EVENT_ORIENTATIONCHANGE, &oce, 0);
  }

  int newOrientation = (oce.orientationIndex == EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY
  || oce.orientationIndex == EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY) ? EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY : EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY;
  // Test locking of orientation.
  ret = emscripten_lock_orientation(newOrientation);
  TEST_RESULT(emscripten_lock_orientation);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("Locked orientation to state %d.\n", newOrientation);
  }

  ret = emscripten_get_orientation_status(&oce);
  TEST_RESULT(emscripten_get_orientation_status);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("The current orientation is after locking:\n");
    orientationchange_callback(18, &oce, 0);
  }

  ret = emscripten_unlock_orientation();
  TEST_RESULT(emscripten_unlock_orientation);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("Unlocked orientation.\n");
  }

  EmscriptenFullscreenChangeEvent fsce;
  ret = emscripten_get_fullscreen_status(&fsce);
  TEST_RESULT(emscripten_get_fullscreen_status);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("The current fullscreen status is:\n");
    fullscreenchange_callback(EMSCRIPTEN_EVENT_FULLSCREENCHANGE, &fsce, 0);
  }

  ret = emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, fullscreenchange_callback);
  TEST_RESULT(emscripten_set_fullscreenchange_callback);

  // These won't do anything, since fullscreen must be requested in an event handler,
  // but call these anyways to confirm that they don't crash in an exception in the test suite.
  ret = emscripten_request_fullscreen("#canvas", 1);
  TEST_RESULT(emscripten_request_fullscreen);
  ret = emscripten_exit_fullscreen();
  TEST_RESULT(emscripten_exit_fullscreen);

  EmscriptenPointerlockChangeEvent plce;
  ret = emscripten_get_pointerlock_status(&plce);
  TEST_RESULT(emscripten_get_pointerlock_status);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("The current pointerlock status is:\n");
    pointerlockchange_callback(EMSCRIPTEN_EVENT_POINTERLOCKCHANGE, &plce, 0);
  }

  ret = emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, pointerlockchange_callback);
  TEST_RESULT(emscripten_set_pointerlockchange_callback);

  // These won't do anything, since pointer lock must be requested in an event handler,
  // but call these anyways to confirm that they don't crash in an exception in the test suite.
  ret = emscripten_request_pointerlock("#canvas", 1);
  TEST_RESULT(emscripten_request_pointerlock);
  ret = emscripten_exit_pointerlock();
  TEST_RESULT(emscripten_exit_pointerlock);

  int vibratePattern[] = {
    150, 500,
    300, 500,
    450
  };
  ret = emscripten_vibrate_pattern(vibratePattern, sizeof(vibratePattern)/sizeof(vibratePattern[0]));
  TEST_RESULT(emscripten_vibrate_pattern);

  EmscriptenVisibilityChangeEvent vce;
  ret = emscripten_get_visibility_status(&vce);
  TEST_RESULT(emscripten_get_visibility_status);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("Current visibility status:\n");
    visibilitychange_callback(EMSCRIPTEN_EVENT_VISIBILITYCHANGE, &vce, 0);
  }

  ret = emscripten_set_visibilitychange_callback(0, 1, visibilitychange_callback);
  TEST_RESULT(emscripten_set_visibilitychange_callback);

  ret = emscripten_set_touchstart_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touch_callback);
  TEST_RESULT(emscripten_set_touchstart_callback);
  ret = emscripten_set_touchend_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touch_callback);
  TEST_RESULT(emscripten_set_touchend_callback);
  ret = emscripten_set_touchmove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touch_callback);
  TEST_RESULT(emscripten_set_touchmove_callback);
  ret = emscripten_set_touchcancel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touch_callback);
  TEST_RESULT(emscripten_set_touchcancel_callback);

  ret = emscripten_set_beforeunload_callback(0, beforeunload_callback);
  TEST_RESULT(emscripten_set_beforeunload_callback);

  ret = emscripten_set_batterychargingchange_callback(0, battery_callback);
  TEST_RESULT(emscripten_set_batterychargingchange_callback);
  ret = emscripten_set_batterylevelchange_callback(0, battery_callback);
  TEST_RESULT(emscripten_set_batterylevelchange_callback);

  EmscriptenBatteryEvent bs;
  ret = emscripten_get_battery_status(&bs);
  TEST_RESULT(emscripten_get_battery_status);
  if (ret == EMSCRIPTEN_RESULT_SUCCESS) {
    printf("Current battery status:\n");
    battery_callback(EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE, &bs, 0);
  }

  ret = emscripten_set_webglcontextlost_callback("#canvas", 0, 1, webglcontext_callback);
  TEST_RESULT(emscripten_set_webglcontextlost_callback);
  ret = emscripten_set_webglcontextrestored_callback("#canvas", 0, 1, webglcontext_callback);
  TEST_RESULT(emscripten_set_webglcontextrestored_callback);

  char *source_window_title = "test window title Äあ🙂";
  emscripten_set_window_title(source_window_title);
  char *current_window_title = emscripten_get_window_title();
  ret = (strcmp(source_window_title, current_window_title) == 0 \
                  ? EMSCRIPTEN_RESULT_SUCCESS : EMSCRIPTEN_RESULT_FAILED);
  TEST_RESULT(emscripten_get_window_title);

  int width, height;
  emscripten_get_screen_size(&width, &height);
  ret = (width && height) ? EMSCRIPTEN_RESULT_SUCCESS : EMSCRIPTEN_RESULT_FAILED;
  TEST_RESULT(emscripten_get_screen_size);

#ifdef KEEP_ALIVE
  emscripten_exit_with_live_runtime();
#else
  // Keep the page running for a moment.
  emscripten_async_call(test_done, 0, 5000);
#endif
  return 0;
}
