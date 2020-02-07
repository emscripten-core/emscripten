// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Make sure these headers exist.
#include <webgpu/webgpu.h>

#include <emscripten.h>
#include <emscripten/html5.h>

int main() {
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
