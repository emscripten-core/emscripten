/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Tests that the memory profiler (--memoryprofiler) tracks the end of the
// dynamic memory area (i.e. the sbrk() limit) correctly, and that its UI keeps
// working after the runtime has exited.

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/heap.h>

EM_JS(void, check_heap_end, (double expected), {
  var actual = emscriptenMemoryProfiler.heapEnd;
  assert(actual == expected, `heapEnd is ${actual}, expected ${expected}`);
  globalThis.expectedHeapEnd = expected;
});

// The memory profiler UI keeps updating on a timer after the runtime has
// exited.  onExit gets called once the runtime has exited, so update the UI
// from there, the same way the timer would, and verify the heap end that the
// UI then displays.
EM_JS(void, verify_ui_after_exit, (void), {
  Module.onExit = (status) => {
    emscriptenMemoryProfiler.updateUi();
    var html = document.getElementById('memoryprofiler_summary').innerHTML;
    var match = /DYNAMIC memory area size: .*? end: 0x([0-9a-f]+)[.]/.exec(html);
    assert(match, 'heap end not found in: ' + html);
    var displayed = parseInt(match[1], 16);
    assert(displayed == expectedHeapEnd, `UI shows heap end ${displayed}, expected ${expectedHeapEnd}`);
    reportResultToServer(`exit:${status}`);
  };
});

void check(void) {
  // Read the sbrk limit directly, since calling sbrk(0) would itself report
  // the limit to the memory profiler.
  check_heap_end((double)*emscripten_get_sbrk_ptr());
}

int main(void) {
  verify_ui_after_exit();

  void* p = malloc(1024);
  assert(p);
  check();

  // Grow the dynamic memory area.
  void* q = malloc(4 * 1024 * 1024);
  assert(q);
  check();

  free(q);
  check();

  free(p);
  return 0;
}
