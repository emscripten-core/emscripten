// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

bool wheel_cb(int eventType, const EmscriptenWheelEvent *wheelEvent __attribute__((nonnull)), void *userData) { return true; }
bool key_cb(int eventType, const EmscriptenKeyboardEvent *e, void *userData) { return true; }
bool mouse_cb(int eventType, const EmscriptenMouseEvent *mouseEvent __attribute__((nonnull)), void *userData) { return true; }
bool ui_cb(int eventType, const EmscriptenUiEvent *uiEvent __attribute__((nonnull)), void *userData) { return true; }
bool focus_cb(int eventType, const EmscriptenFocusEvent *focusEvent __attribute__((nonnull)), void *userData) { return true; }
bool fullscreenchange_cb(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent __attribute__((nonnull)), void *userData) { return true; }
bool pointerlockchange_cb(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent __attribute__((nonnull)), void *userData) { return true; }
bool pointerlockerror_cb(int eventType, const void *reserved, void *userData) { return true; }
bool touch_cb(int eventType, const EmscriptenTouchEvent *touchEvent __attribute__((nonnull)), void *userData) { return true; }

int main(int argc, char **argv)
{
  assert(emscripten_set_keypress_callback("this_dom_element_does_not_exist", 0, 0, key_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_keydown_callback("this_dom_element_does_not_exist", 0, 0, key_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_keyup_callback("this_dom_element_does_not_exist", 0, 0, key_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_click_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mousedown_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mouseup_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_dblclick_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mousemove_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mouseenter_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mouseleave_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mouseover_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_mouseout_callback("this_dom_element_does_not_exist", 0, 0, mouse_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_wheel_callback("this_dom_element_does_not_exist", 0, 1, wheel_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_resize_callback("this_dom_element_does_not_exist", 0, 1, ui_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_scroll_callback("this_dom_element_does_not_exist", 0, 1, ui_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_blur_callback("this_dom_element_does_not_exist", 0, 1, focus_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_focus_callback("this_dom_element_does_not_exist", 0, 1, focus_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_focusin_callback("this_dom_element_does_not_exist", 0, 1, focus_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_focusout_callback("this_dom_element_does_not_exist", 0, 1, focus_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_fullscreenchange_callback("this_dom_element_does_not_exist", 0, 1, fullscreenchange_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_pointerlockchange_callback("this_dom_element_does_not_exist", 0, 1, pointerlockchange_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_pointerlockerror_callback("this_dom_element_does_not_exist", 0, 1, pointerlockerror_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);

  assert(emscripten_set_touchstart_callback("this_dom_element_does_not_exist", 0, 1, touch_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_touchend_callback("this_dom_element_does_not_exist", 0, 1, touch_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_touchmove_callback("this_dom_element_does_not_exist", 0, 1, touch_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  assert(emscripten_set_touchcancel_callback("this_dom_element_does_not_exist", 0, 1, touch_cb) == EMSCRIPTEN_RESULT_UNKNOWN_TARGET);
  return 0;
}
