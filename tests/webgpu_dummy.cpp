// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Make sure this header exists and compiles
// (webgpu_cpp.h includes webgpu.h so that's tested too).
#include <webgpu/webgpu_cpp.h>

#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

int main() {
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
