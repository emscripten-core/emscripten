/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if STACK_OVERFLOW_CHECK
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
#if RUNTIME_DEBUG
  dbg(`writeStackCookie: ${ptrToString(max)}`);
#endif
#if ASSERTIONS
  assert((max & 3) == 0);
#endif
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  {{{ makeSetValue('max', 0, '0x02135467', 'u32') }}};
  {{{ makeSetValue('max', 4, '0x89BACDFE', 'u32') }}};
#if CHECK_NULL_WRITES
  // Also test the global address 0 for integrity.
  {{{ makeSetValue(0, 0, 0x63736d65 /* 'emsc' */, 'u32') }}};
#endif
}

function checkStackCookie() {
#if !MINIMAL_RUNTIME
  if (ABORT) return;
#endif
  var max = _emscripten_stack_get_end();
#if RUNTIME_DEBUG
  dbg(`checkStackCookie: ${ptrToString(max)}`);
#endif
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var cookie1 = {{{ makeGetValue('max', 0, 'u32') }}};
  var cookie2 = {{{ makeGetValue('max', 4, 'u32') }}};
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${ptrToString(cookie2)} ${ptrToString(cookie1)}`);
  }
#if CHECK_NULL_WRITES
  // Also test the global address 0 for integrity.
  if ({{{ makeGetValue(0, 0, 'u32') }}} != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
#endif
}
#endif
