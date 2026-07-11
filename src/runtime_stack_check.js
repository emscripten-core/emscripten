/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !STACK_OVERFLOW_CHECK
#error "should only be included in STACK_OVERFLOW_CHECK mode"
#endif

const stackCookie1 = 0x02135467;
const stackCookie2 = 0x89BACDFE;

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
  {{{ makeSetValue('max', 0, 'stackCookie1', 'u32') }}};
  {{{ makeSetValue('max', 4, 'stackCookie2', 'u32') }}};
#if CHECK_NULL_WRITES
  // Also test the global address 0 for integrity.
  {{{ makeSetValue(0, 0, 0x63736d65 /* 'emsc' */, 'u32') }}};
#endif
}

function u32ToHexString(num) {
  return '0x' + (num >>> 0).toString(16).padStart(8, '0');
}

function checkStackCookie() {
#if !MINIMAL_RUNTIME
  if (ABORT) return;
#endif
  var max = _emscripten_stack_get_end();
#if RUNTIME_DEBUG >= 2
  dbg(`checkStackCookie: ${ptrToString(max)}`);
#endif
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var val1 = {{{ makeGetValue('max', 0, 'u32') }}};
  var val2 = {{{ makeGetValue('max', 4, 'u32') }}};
  if (val1 != stackCookie1 || val2 != stackCookie2) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords ${u32ToHexString(stackCookie2)} and ${u32ToHexString(stackCookie1)}, but received ${u32ToHexString(val2)} ${u32ToHexString(val1)}`);
  }
#if CHECK_NULL_WRITES
  // Also test the global address 0 for integrity.
  if ({{{ makeGetValue(0, 0, 'u32') }}} != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
#endif
}
