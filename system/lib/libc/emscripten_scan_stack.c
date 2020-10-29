/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/stack.h>

void emscripten_scan_stack(em_scan_func func) {
  uintptr_t base = emscripten_stack_get_base();
  uintptr_t end = emscripten_stack_get_current();
  func((void*)end, (void*)base);
}
