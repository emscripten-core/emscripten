// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/html5.h>

static int result = 1;

// When running PROXY_TO_WORKER mode that return code of key event handlers
// is not honored (since the handlers are run asyncronously in the worker).
// Instead `shouldPreventDefault` in `proxyClient.js` returns true for all keys
// except backspace and tab.
//
// Therefore where we expect the keypress callback to be prevented for '\b'
// but not for 'A'.

bool keydown_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  printf("got keydown: %d\n", e->keyCode);
  if ((e->keyCode == 'A') || (e->keyCode == '\b')) {
    result *= 2;
  } else {
    printf("done: result=%d\n", result);
    emscripten_force_exit(result);
  }
  return 0;
}

bool keypress_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  printf("got keypress: %d\n", e->keyCode);
  // preventDefault should have been set for the backspace key so we should
  // never get that here.
  assert(e->keyCode != '\b');
  result *= 3;
  return 0;
}

bool keyup_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  printf("got keyup: %d\n", e->keyCode);
  if ((e->keyCode == 'A') || (e->keyCode == '\b')) {
    result *= 5;
  }
  return 0;
}

int main(int argc, char **argv) {
  printf("in main\n");

  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, keydown_callback);
  emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, keypress_callback);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, keyup_callback);

  emscripten_runtime_keepalive_push();
  return 0;
}

