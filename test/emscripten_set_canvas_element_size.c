/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// Tests the operation of emscripten_set_canvas_element_size() and emscripten_get_canvas_element_size()

int main(int argc, char **argv) {
  // For testing purposes, rename the canvas on the page to some arbitrary ID.
  EM_ASM(document.getElementById('canvas').id = 'myCanvasId');

  // Test emscripten_set_canvas_element_size()
  EMSCRIPTEN_RESULT r = emscripten_set_canvas_element_size("#myCanvasId", 100, 200);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  int w, h;
  r = emscripten_get_canvas_element_size("#myCanvasId", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 100);
  assert(h == 200);
  w = h = 0;

  // The following line will not work with OffscreenCanvas, that is covered in another test
  int jsAgreesWithSize = EM_ASM_INT({return document.querySelector('#myCanvasId').width == 100 && document.querySelector('#myCanvasId').height == 200});
  assert(jsAgreesWithSize);

  // Accessing by specific ID should resize canvas by that ID
  r = emscripten_set_canvas_element_size("#myCanvasId", 102, 202);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // Check that we see the change on the canvas when querying with ID
  r = emscripten_get_canvas_element_size("#myCanvasId", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 102);
  assert(h == 202);

  // Check that we see the change on the canvas when querying with DOM element type
  r = emscripten_get_canvas_element_size("canvas", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 102);
  assert(h == 202);
  w = h = 0;

  return 0;
}
