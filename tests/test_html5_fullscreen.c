#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>

#ifdef REPORT_RESULT
void report_result(int result)
{
  if (result == 0) {
    printf("Test successful!\n");
  } else {
    printf("Test failed!\n");
  }
  REPORT_RESULT();
}
#endif

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

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "f") || e->which == 102)) {
    EmscriptenFullscreenChangeEvent fsce;
    EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);
    TEST_RESULT(emscripten_get_fullscreen_status);
    if (!fsce.isFullscreen) {
      printf("Requesting fullscreen..\n");
      ret = emscripten_request_fullscreen(0, 1);
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

  return 0;
}

int callCount = 0;

EM_BOOL fullscreenchange_callback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData)
{
  printf("%s, isFullscreen: %d, fullscreenEnabled: %d, fs element nodeName: \"%s\", fs element id: \"%s\". New size: %dx%d pixels. Screen size: %dx%d pixels.\n",
    emscripten_event_type_to_string(eventType), e->isFullscreen, e->fullscreenEnabled, e->nodeName, e->id, e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);

  ++callCount;
  if (callCount == 1) { // Transitioned to fullscreen.
    if (!e->isFullscreen) {
      report_result(1);
    }
  } else if (callCount == 2) { // Transitioned to windowed, we must be back to the default pixel size 300x150.
    if (e->isFullscreen || e->elementWidth != 300 || e->elementHeight != 150) {
      report_result(1);
    } else {
      report_result(0);
    }
  }
  return 0;
}

int main()
{
  EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(0, 0, 1, key_callback);
  TEST_RESULT(emscripten_set_keypress_callback);

  ret = emscripten_set_fullscreenchange_callback(0, 0, 1, fullscreenchange_callback);
  TEST_RESULT(emscripten_set_fullscreenchange_callback);

  printf("To finish this test, press f to enter fullscreen mode, and then exit it.\n");

  /* For the events to function, one must either call emscripten_set_main_loop or enable Module.noExitRuntime by some other means. 
     Otherwise the application will exit after leaving main(), and the atexit handlers will clean up all event hooks (by design). */
  EM_ASM(Module['noExitRuntime'] = true);
  return 0;
}
