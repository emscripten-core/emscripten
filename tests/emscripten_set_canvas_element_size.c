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

int main(int argc, char **argv)
{
  // For testing purposes, rename the canvas on the page to some arbitrary ID.
  EM_ASM(document.getElementById('canvas').id = 'myCanvasId');

  // Accessing #canvas should resize Module['canvas']
  EMSCRIPTEN_RESULT r = emscripten_set_canvas_element_size("#canvas", 100, 200);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  int w, h;
  r = emscripten_get_canvas_element_size("#canvas", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 100);
  assert(h == 200);
  w = h = 0;

  // Check that we see the change via 'NULL'
  r = emscripten_get_canvas_element_size(NULL, &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 100);
  assert(h == 200);
  w = h = 0;

  // Check that we see the change via 'mycanvasId'
  r = emscripten_get_canvas_element_size("myCanvasId", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 100);
  assert(h == 200);

  // The following line will not work with OffscreenCanvas, that is covered in another test
  int jsAgreesWithSize = EM_ASM_INT({return Module['canvas'].width == 100 && Module['canvas'].height == 200});
  assert(jsAgreesWithSize);

  // Accessing NULL should also resize Module['canvas']
  r = emscripten_set_canvas_element_size(NULL, 101, 201);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // Check that we see the change on the canvas (via the established #canvas)
  r = emscripten_get_canvas_element_size("#canvas", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 101);
  assert(h == 201);
  w = h = 0;

  // Check that we see the change via 'NULL'
  r = emscripten_get_canvas_element_size(NULL, &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 101);
  assert(h == 201);
  w = h = 0;

  // Check that we see the change via 'mycanvasId'
  r = emscripten_get_canvas_element_size("myCanvasId", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 101);
  assert(h == 201);

  // Accessing by specific ID should resize canvas by that ID
  r = emscripten_set_canvas_element_size("myCanvasId", 102, 202);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // Check that we see the change on the canvas (via the established #canvas)
  r = emscripten_get_canvas_element_size("#canvas", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 102);
  assert(h == 202);

  // Check that we see the change on the canvas (via the established #canvas)
  r = emscripten_get_canvas_element_size("#canvas", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 102);
  assert(h == 202);
  w = h = 0;

  // Check that we see the change via 'NULL'
  r = emscripten_get_canvas_element_size(NULL, &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 102);
  assert(h == 202);
  w = h = 0;

  // Check that we see the change via 'mycanvasId'
  r = emscripten_get_canvas_element_size("myCanvasId", &w, &h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(w == 102);
  assert(h == 202);

#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}
