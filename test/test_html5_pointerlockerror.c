/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
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

bool click_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  if (e->screenX != 0 && e->screenY != 0 && e->clientX != 0 && e->clientY != 0 && e->canvasX != 0 && e->canvasY != 0 && e->targetX != 0 && e->targetY != 0)
  {
    if (eventType == EMSCRIPTEN_EVENT_CLICK && !gotClick) {
      gotClick = 1;
      printf("Request pointer lock...\n");
      EMSCRIPTEN_RESULT ret = emscripten_request_pointerlock(0, 0);
      TEST_RESULT(ret);
      if (ret != EMSCRIPTEN_RESULT_SUCCESS) {
        printf("ERROR! emscripten_request_pointerlock() failure\n");
        report_result(1);
      }
    }
  }

  return 0;
}

bool pointerlockchange_callback(int eventType, const EmscriptenPointerlockChangeEvent *e, void *userData) {
  printf("ERROR! received 'pointerlockchange' event\n");
  report_result(1);

  return 0;
}

bool pointerlockerror_callback(int eventType, const void *reserved, void *userData) {
  if (eventType != EMSCRIPTEN_EVENT_POINTERLOCKERROR) {
    printf("ERROR! invalid event type for 'pointerlockerror' callback\n");
    report_result(1);
    return 0;
  }

  printf("SUCCESS! received 'pointerlockerror' event\n");
  report_result(0);

  return 0;
}

int main() {
  printf("'pointerlockerror' event test:\n");
  printf("Reject the pointer lock request after clicking on canvas.\n");

  // Make the canvas area stand out from the background.
  emscripten_set_canvas_element_size( "#canvas", 400, 300 );
  EM_ASM(Module['canvas'].style.backgroundColor = 'black';);

  EMSCRIPTEN_RESULT ret = emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, click_callback);
  TEST_RESULT(emscripten_set_click_callback);
  ret = emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, pointerlockchange_callback);
  TEST_RESULT(emscripten_set_pointerlockchange_callback);
  ret = emscripten_set_pointerlockerror_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, pointerlockerror_callback);
  TEST_RESULT(emscripten_set_pointerlockerror_callback);

  emscripten_exit_with_live_runtime();
  return 0;
}
