#if STACK_OVERFLOW_CHECK
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  assert((STACK_MAX & 3) == 0);
#if WASM_BACKEND
  // The stack grows downwards
  HEAPU32[(STACK_MAX >> 2)+1] = 0x02135467;
  HEAPU32[(STACK_MAX >> 2)+2] = 0x89BACDFE;
#else
  HEAPU32[(STACK_MAX >> 2)-1] = 0x02135467;
  HEAPU32[(STACK_MAX >> 2)-2] = 0x89BACDFE;
#endif
}

function checkStackCookie() {
#if WASM_BACKEND
  var cookie1 = HEAPU32[(STACK_MAX >> 2)+1];
  var cookie2 = HEAPU32[(STACK_MAX >> 2)+2];
#else
  var cookie1 = HEAPU32[(STACK_MAX >> 2)-1];
  var cookie2 = HEAPU32[(STACK_MAX >> 2)-2];
#endif
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort('Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x02135467, but received 0x' + cookie2.toString(16) + ' ' + cookie1.toString(16));
  }
#if !USE_ASAN
  // Also test the global address 0 for integrity.
  // We don't do this with ASan because ASan does its own checks for this.
  if (HEAP32[0] !== 0x63736d65 /* 'emsc' */) abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
#endif
}

#if !MINIMAL_RUNTIME // MINIMAL_RUNTIME moves this to a JS library function
function abortStackOverflow(allocSize) {
  abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (STACK_MAX - stackSave() + allocSize) + ' bytes available!');
}
#endif

#endif

#if STACK_OVERFLOW_CHECK && !USE_ASAN
#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  HEAP32[0] = 0x63736d65; /* 'emsc' */
#if USE_PTHREADS
} else {
  if (HEAP32[0] !== 0x63736d65) abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
}
#endif // USE_PTHREADS
#endif // STACK_OVERFLOW_CHECK
