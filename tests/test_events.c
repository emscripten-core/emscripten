#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/events.h>

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

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
EM_BOOL key_callback(int eventType, const emscripten_KeyboardEvent *e, void *userData)
{
  printf("%s, key: \"%s\", code: \"%s\", location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu, keyCode: %lu, which: %lu\n",
    emscripten_event_type_to_string(eventType), e->key, e->code, e->location, 
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
    e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which);

  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && !strcmp(e->key, "f")) {
    emscripten_FullscreenChangeEvent fsce;
    emscripten_get_fullscreen_status(&fsce);
    if (!fsce.isFullscreen) {
      emscripten_request_fullscreen(0, 1);
    } else {
      emscripten_exit_fullscreen();
    }
  }

  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && !strcmp(e->key, "p")) {
    emscripten_PointerlockChangeEvent plce;
    emscripten_get_pointerlock_status(&plce);
    if (!plce.isActive) {
      emscripten_request_pointerlock(0, 1);
    } else {
      emscripten_exit_pointerlock();
    }
  }

  return 0;
}

EM_BOOL mouse_callback(int eventType, const emscripten_MouseEvent *e, void *userData)
{
  printf("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, movement: (%ld,%ld), canvas: (%ld,%ld)\n",
    emscripten_event_type_to_string(eventType), e->screenX, e->screenY, e->clientX, e->clientY,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
    e->button, e->buttons, e->movementX, e->movementY, e->canvasX, e->canvasY);

  return 0;
}

EM_BOOL wheel_callback(int eventType, const emscripten_WheelEvent *e, void *userData)
{
  printf("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, canvas: (%ld,%ld), delta:(%g,%g,%g), deltaMode:%lu\n",
    emscripten_event_type_to_string(eventType), e->mouse.screenX, e->mouse.screenY, e->mouse.clientX, e->mouse.clientY,
    e->mouse.ctrlKey ? " CTRL" : "", e->mouse.shiftKey ? " SHIFT" : "", e->mouse.altKey ? " ALT" : "", e->mouse.metaKey ? " META" : "", 
    e->mouse.button, e->mouse.buttons, e->mouse.canvasX, e->mouse.canvasY,
    (float)e->deltaX, (float)e->deltaY, (float)e->deltaZ, e->deltaMode);

  return 0;
}

EM_BOOL uievent_callback(int eventType, const emscripten_UiEvent *e, void *userData)
{
  printf("%s, detail: %ld, document.body.client size: (%d,%d), window.inner size: (%d,%d), scrollPos: (%d, %d)\n",
    emscripten_event_type_to_string(eventType), e->detail, e->documentBodyClientWidth, e->documentBodyClientHeight,
    e->windowInnerWidth, e->windowInnerHeight, e->scrollTop, e->scrollLeft);

  return 0;
}

EM_BOOL focusevent_callback(int eventType, const emscripten_FocusEvent *e, void *userData)
{
  printf("%s, nodeName: \"%s\", id: \"%s\"\n", emscripten_event_type_to_string(eventType), e->nodeName, e->id[0] == '\0' ? "(empty string)" : e->id);

  return 0;
}

EM_BOOL deviceorientation_callback(int eventType, const emscripten_DeviceOrientationEvent *e, void *userData)
{
  printf("%s, (%g, %g, %g)\n", emscripten_event_type_to_string(eventType), e->alpha, e->beta, e->gamma);

  return 0;
}

EM_BOOL devicemotion_callback(int eventType, const emscripten_DeviceMotionEvent *e, void *userData)
{
  printf("%s, accel: (%g, %g, %g), accelInclGravity: (%g, %g, %g), rotationRate: (%g, %g, %g)\n", 
    emscripten_event_type_to_string(eventType), 
    e->accelerationX, e->accelerationY, e->accelerationZ,
    e->accelerationIncludingGravityX, e->accelerationIncludingGravityY, e->accelerationIncludingGravityZ,
    e->rotationRateAlpha, e->rotationRateBeta, e->rotationRateGamma);

  return 0;
}

EM_BOOL orientationchange_callback(int eventType, const emscripten_OrientationChangeEvent *e, void *userData)
{
  printf("%s, orientationAngle: %d, orientationIndex: %d\n", emscripten_event_type_to_string(eventType), e->orientationAngle, e->orientationIndex);

  return 0;
}

EM_BOOL fullscreenchange_callback(int eventType, const emscripten_FullscreenChangeEvent *e, void *userData)
{
  printf("%s, isFullscreen: %d, fullscreenEnabled: %d, fs element nodeName: \"%s\", fs element id: \"%s\"\n",
    emscripten_event_type_to_string(eventType), e->isFullscreen, e->fullscreenEnabled, e->nodeName, e->id);

  return 0;
}

EM_BOOL pointerlockchange_callback(int eventType, const emscripten_PointerlockChangeEvent *e, void *userData)
{
  printf("%s, isActive: %d, pointerlock element nodeName: \"%s\", id: \"%s\"\n",
    emscripten_event_type_to_string(eventType), e->isActive, e->nodeName, e->id);

  return 0;
}

EM_BOOL visibilitychange_callback(int eventType, const emscripten_VisibilityChangeEvent *e, void *userData)
{
  printf("%s, hidden: %d, visibilityState: %d\n", emscripten_event_type_to_string(eventType), e->hidden, e->visibilityState);

  return 0;
}

EM_BOOL touch_callback(int eventType, const emscripten_TouchEvent *e, void *userData)
{
  printf("%s, numTouches: %d %s%s%s%s\n",
    emscripten_event_type_to_string(eventType), e->numTouches,
    e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "");
  for(int i = 0; i < e->numTouches; ++i)
  {
    const emscripten_TouchPoint *t = &e->touches[i];
    printf("  %ld: screen: (%ld,%ld), client: (%ld,%ld), page: (%ld,%ld), isChanged: %d, onTarget: %d, canvas: (%ld, %ld)\n",
      t->identifier, t->screenX, t->screenY, t->clientX, t->clientY, t->pageX, t->pageY, t->isChanged, t->onTarget, t->canvasX, t->canvasY);
  }
  
  return 0;
}

EM_BOOL gamepad_callback(int eventType, const emscripten_GamepadEvent *e, void *userData)
{
  printf("%s: timeStamp: %g, connected: %d, index: %ld, numAxes: %d, numButtons: %d, id: \"%s\", mapping: \"%s\"\n",
    eventType != 0 ? emscripten_event_type_to_string(eventType) : "Gamepad state", e->timestamp, e->connected, e->index, 
    e->numAxes, e->numButtons, e->id, e->mapping);

  if (e->connected)
  {
    for(int i = 0; i < e->numAxes; ++i)
      printf("Axis %d: %g\n", i, e->axis[i]);

    for(int i = 0; i < e->numButtons; ++i)
      printf("Button %d: Digital: %d, Analog: %g\n", i, e->digitalButton[i], e->analogButton[i]);
  }

  return 0;
}

const char *beforeunload_callback(int eventType, const void *reserved, void *userData)
{
#ifdef REPORT_RESULT
  return ""; // For test harness, don't show a confirmation dialog to not block and keep the test runner automated.
#else
  return "Do you really want to leave the page?";
#endif
}

void formatTime(char *str, int seconds)
{
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

EM_BOOL battery_callback(int eventType, const emscripten_BatteryEvent *e, void *userData)
{
  char t1[64];
  formatTime(t1, (int)e->chargingTime);
  char t2[64];
  formatTime(t2, (int)e->dischargingTime);
  printf("%s: chargingTime: %s, dischargingTime: %s, level: %g%%, charging: %d\n",
    emscripten_event_type_to_string(eventType), t1, t2, e->level*100, e->charging);

  return 0;
}

EM_BOOL webglcontext_callback(int eventType, const void *reserved, void *userData)
{
  printf("%s.\n", emscripten_event_type_to_string(eventType));

  return 0;
}

emscripten_GamepadEvent prevState[32];
int prevNumGamepads = 0;

void mainloop()
{
  int numGamepads = emscripten_get_num_gamepads();
  if (numGamepads != prevNumGamepads)
  {
    printf("Number of connected gamepads: %d\n", numGamepads);
    prevNumGamepads = numGamepads;
  }

  for(int i = 0; i < numGamepads && i < 32; ++i)
  {
    emscripten_GamepadEvent ge;
    int failed = emscripten_get_gamepad_status(i, &ge);
    if (!failed)
    {
      int g = ge.index;
      for(int j = 0; j < ge.numAxes; ++j)
      {
        if (ge.axis[j] != prevState[g].axis[j])
          printf("Gamepad %d, axis %d: %g\n", g, j, ge.axis[j]);
      }

      for(int j = 0; j < ge.numButtons; ++j)
      {
        if (ge.analogButton[j] != prevState[g].analogButton[j] || ge.digitalButton[j] != prevState[g].digitalButton[j])
          printf("Gamepad %d, button %d: Digital: %d, Analog: %g\n", g, j, ge.digitalButton[j], ge.analogButton[j]);
      }
      prevState[g] = ge;
    }
  }

}

#ifdef REPORT_RESULT
void report_result(void *arg)
{
  int result = 0;
  REPORT_RESULT();
}
#endif

int main()
{
  emscripten_set_keypress_callback(0, 0, 1, key_callback);
  emscripten_set_keydown_callback(0, 0, 1, key_callback);
  emscripten_set_keyup_callback(0, 0, 1, key_callback);

  emscripten_set_click_callback(0, 0, 1, mouse_callback);
  emscripten_set_mousedown_callback(0, 0, 1, mouse_callback);
  emscripten_set_mouseup_callback(0, 0, 1, mouse_callback);
  emscripten_set_dblclick_callback(0, 0, 1, mouse_callback);
  emscripten_set_mousemove_callback(0, 0, 1, mouse_callback);

  emscripten_set_wheel_callback(0, 0, 1, wheel_callback);

  emscripten_set_resize_callback(0, 0, 1, uievent_callback);
  emscripten_set_scroll_callback(0, 0, 1, uievent_callback);

  emscripten_set_blur_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focus_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focusin_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focusout_callback(0, 0, 1, focusevent_callback);

  emscripten_set_deviceorientation_callback(0, 1, deviceorientation_callback);
  emscripten_set_devicemotion_callback(0, 1, devicemotion_callback);

  emscripten_set_orientationchange_callback(0, 1, orientationchange_callback);

  // Test the polling of orientation.
  emscripten_OrientationChangeEvent oce;
  emscripten_get_orientation_status(&oce);
  printf("The current orientation is:\n");
  orientationchange_callback(EMSCRIPTEN_EVENT_ORIENTATIONCHANGE, &oce, 0);

  int newOrientation = (oce.orientationIndex == EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY 
  || oce.orientationIndex == EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY) ? EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY : EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY;
  printf("Locking orientation to state %d..\n", newOrientation);
  // Test locking of orientation.
  int failed = emscripten_lock_orientation(newOrientation);

  if (failed)
    printf("No support for orientation lock!\n");

  emscripten_get_orientation_status(&oce);
  printf("The current orientation is after locking:\n");
  orientationchange_callback(18, &oce, 0);

  printf("Unlocking orientation..\n");
  emscripten_unlock_orientation();

  emscripten_FullscreenChangeEvent fsce;
  emscripten_get_fullscreen_status(&fsce);
  printf("The current fullscreen status is:\n");
  fullscreenchange_callback(EMSCRIPTEN_EVENT_FULLSCREENCHANGE, &fsce, 0);

  emscripten_set_fullscreenchange_callback(0, 1, fullscreenchange_callback);

  // These won't do anything, since fullscreen must be requested in an event handler,
  // but call these anyways to confirm that they don't crash in an exception in the test suite.
  failed = emscripten_request_fullscreen(0, 1);
  if (failed != 0 && failed != 3)
    printf("Fullscreen request failed! (%d)\n", failed);
  emscripten_exit_fullscreen();

  emscripten_PointerlockChangeEvent plce;
  emscripten_get_pointerlock_status(&plce);
  printf("The current pointerlock status is:\n");
  pointerlockchange_callback(EMSCRIPTEN_EVENT_POINTERLOCKCHANGE, &plce, 0);

  emscripten_set_pointerlockchange_callback(0, 1, pointerlockchange_callback);

  // These won't do anything, since pointer lock must be requested in an event handler,
  // but call these anyways to confirm that they don't crash in an exception in the test suite.
  failed = emscripten_request_pointerlock(0, 1);
  if (failed != 0 && failed != 3)
    printf("Pointer lock request failed! (%d)\n", failed);
  emscripten_exit_pointerlock();

  int vibratePattern[] = {
    150, 500, 
    300, 500, 
    450
  };
  emscripten_vibrate_pattern(vibratePattern, sizeof(vibratePattern)/sizeof(vibratePattern[0]));

  emscripten_VisibilityChangeEvent vce;
  emscripten_get_visibility_status(&vce);
  printf("Current visibility status:\n");
  visibilitychange_callback(EMSCRIPTEN_EVENT_VISIBILITYCHANGE, &vce, 0);

  emscripten_set_visibilitychange_callback(0, 1, visibilitychange_callback);

  emscripten_set_touchstart_callback(0, 0, 1, touch_callback);
  emscripten_set_touchend_callback(0, 0, 1, touch_callback);
  emscripten_set_touchmove_callback(0, 0, 1, touch_callback);
  emscripten_set_touchcancel_callback(0, 0, 1, touch_callback);

  emscripten_set_gamepadconnected_callback(0, 1, gamepad_callback);
  emscripten_set_gamepaddisconnected_callback(0, 1, gamepad_callback);

  emscripten_set_main_loop(mainloop, 10, 0);

  emscripten_set_beforeunload_callback(0, beforeunload_callback);

  emscripten_set_batterychargingchange_callback(0, battery_callback);
  emscripten_set_batterylevelchange_callback(0, battery_callback);

  emscripten_BatteryEvent bs;
  failed = emscripten_get_battery_status(&bs);
  if (failed) {
    printf("Browser can not provide Battery Status information!\n");
  } else {
    printf("Current battery status:\n");
    battery_callback(EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE, &bs, 0);
  }

  emscripten_set_webglcontextlost_callback(0, 0, 1, webglcontext_callback);
  emscripten_set_webglcontextrestored_callback(0, 0, 1, webglcontext_callback);

  /* For the events to function, one must either call emscripten_set_main_loop or enable Module.noExitRuntime by some other means. 
     Otherwise the application will exit after leaving main(), and the atexit handlers will clean up all event hooks (by design). */
  EM_ASM(Module['noExitRuntime'] = true);
  
#ifdef REPORT_RESULT
  // Keep the page running for a moment.
  emscripten_async_call(report_result, 0, 5000);
#endif
  return 0;
}
