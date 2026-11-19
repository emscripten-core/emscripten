/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Tests that the memory profiler (--memoryprofiler) tracks the end of the
// dynamic memory area (i.e. the sbrk() limit) correctly, and that its UI keeps
// working after the runtime has exited.
//
// This test is also run with PROXY_TO_PTHREAD, in which case it is a pthread
// that grows the dynamic memory area, and the notifications from sbrk() get
// forwarded to the main thread, where the memory profiler lives.

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <emscripten.h>
#include <emscripten/heap.h>

// How many times, at 10 msec intervals, to check for the notification from
// sbrk() to arrive on the main thread before giving up.
#define MAX_TRIES 500

// Returns the end of the dynamic memory area that the memory profiler
// currently knows of.
double get_heap_end(void) {
  return MAIN_THREAD_EM_ASM_DOUBLE({
    return emscriptenMemoryProfiler.heapEnd;
  });
}

// The memory profiler UI keeps updating on a timer after the runtime has
// exited.  onExit gets called once the runtime has exited, so update the UI
// from there, the same way the timer would, and verify the heap end that the
// UI then displays.
void verify_ui_after_exit(void) {
  MAIN_THREAD_EM_ASM({
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
}

void check(void) {
  double expected;
  // When running in a pthread, the notification from sbrk() arrives
  // asynchronously on the main thread, so poll for it rather than rely on it
  // having arrived already.
  for (int tries = 0; tries < MAX_TRIES; ++tries) {
    double actual = get_heap_end();
    // Read the sbrk limit directly, since calling sbrk(0) would itself report
    // the limit to the memory profiler.  Read it on each iteration, and only
    // after calling get_heap_end(), so that the comparison stays valid even if
    // calling to the main thread were to allocate memory.
    expected = (double)*emscripten_get_sbrk_ptr();
    if (actual == expected) {
      break;
    }
    usleep(10 * 1000);
  }
  MAIN_THREAD_EM_ASM({
    var actual = emscriptenMemoryProfiler.heapEnd;
    assert(actual == $0, `heapEnd is ${actual}, expected ${$0}`);
    globalThis.expectedHeapEnd = $0;
  }, expected);
}

int main(void) {
  verify_ui_after_exit();

  void* p = malloc(1024);
  assert(p);
  check();

  // Grow the dynamic memory area.
  uintptr_t old_brk = *emscripten_get_sbrk_ptr();
  void* q = malloc(4 * 1024 * 1024);
  assert(q);
  assert(*emscripten_get_sbrk_ptr() > old_brk);
  check();

  free(q);
  check();

  free(p);
  return 0;
}
