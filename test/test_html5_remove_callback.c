/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>

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

// Report API failure
#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

// Like above above but also assert API success
#define ASSERT_RESULT(x) TEST_RESULT(x); assert(ret == EMSCRIPTEN_RESULT_SUCCESS);

char const *userDataToString(void *userData) {
  return userData ? (char const *) userData : "nullptr";
}

bool key_callback_1(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  printf("key_callback_1: eventType=%d, userData=%s\n", eventType, userDataToString(userData));
  return 0;
}

bool key_callback_2(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  printf("key_callback_2: eventType=%d, userData=%s\n", eventType, userDataToString(userData));
  return 0;
}

bool mouse_callback_1(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  printf("mouse_callback_1: eventType=%d, userData=%s\n", eventType, userDataToString(userData));
  return 0;
}

void checkCount(int count)
{
  int eventHandlersCount = EM_ASM_INT({ return JSEvents.eventHandlers.length; });
  printf("Detected [%d] handlers\n", eventHandlersCount);
  assert(count == eventHandlersCount);
}

void test_done() {}

int main() {
  bool useCapture = true;
  void *userData3 = "3";

  // first we check for invalid parameters
  assert(emscripten_remove_callback("this_dom_element_does_not_exist", NULL, 0, key_callback_1) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  checkCount(0);

  EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, useCapture, key_callback_1);
  ASSERT_RESULT(emscripten_set_keypress_callback);
  ret = emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, useCapture, key_callback_1);
  ASSERT_RESULT(emscripten_set_keydown_callback);
  ret = emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, useCapture, key_callback_1);
  ASSERT_RESULT(emscripten_set_keyup_callback);

  checkCount(3);

  // removing keydown event
  ret = emscripten_remove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EMSCRIPTEN_EVENT_KEYDOWN, key_callback_1);
  ASSERT_RESULT(emscripten_remove_callback);

  checkCount(2);

  // adding another keypress callback on the same target
  ret = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, useCapture, key_callback_2);
  ASSERT_RESULT(emscripten_set_keypress_callback);

  checkCount(3);

  // adding another keypress callback on the same target with different user data
  ret = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, userData3, useCapture, key_callback_2);
  ASSERT_RESULT(emscripten_set_keypress_callback);

  checkCount(4);

  // removing a combination that does not exist (no mouse_callback_1 registered)
  ret = emscripten_remove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EMSCRIPTEN_EVENT_KEYPRESS, mouse_callback_1);
  ASSERT_RESULT(emscripten_remove_callback);

  checkCount(4);

  // removing keypress / userData=NULL / key_callback_2
  ret = emscripten_remove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EMSCRIPTEN_EVENT_KEYPRESS, key_callback_2);
  ASSERT_RESULT(emscripten_remove_callback);

  checkCount(3);

  // removing keypress / userData=NULL / key_callback_1
  ret = emscripten_remove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EMSCRIPTEN_EVENT_KEYPRESS, key_callback_1);
  ASSERT_RESULT(emscripten_remove_callback);

  checkCount(2);

  // removing keypress / userData=3 / key_callback_2
  ret = emscripten_remove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, userData3, EMSCRIPTEN_EVENT_KEYPRESS, key_callback_2);
  ASSERT_RESULT(emscripten_remove_callback);

  checkCount(1);

  // adding the same mouse down callback to 2 different targets
  ret = emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, useCapture, mouse_callback_1);
  ASSERT_RESULT(emscripten_set_mousedown_callback);
  ret = emscripten_set_mousedown_callback("#canvas", NULL, useCapture, mouse_callback_1);
  ASSERT_RESULT(emscripten_set_mousedown_callback);

  checkCount(3);

  // removing mousedown / userData=NULL / mouse_callback_1 on the window target
  ret = emscripten_remove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EMSCRIPTEN_EVENT_MOUSEDOWN, mouse_callback_1);
  ASSERT_RESULT(emscripten_remove_callback);

  checkCount(2);

  return 0;
}
