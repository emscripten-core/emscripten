/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <emscripten/html5.h>

int main() {
  int result = 0;
  double w, h;
  emscripten_get_element_css_size("#canvas", &w, &h);
  if (isnan(w) || isnan(h)) {
    result = 1;
  }
  REPORT_RESULT(result);
  return 0;
}
