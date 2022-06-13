// stores
HEAP8[x] = 1;
HEAP16[x] = 2;
HEAP32[x] = 3;
HEAPU8[x] = 4;
HEAPU16[x] = 5;
HEAPU32[x] = 6;
HEAPF32[x] = 7;
HEAPF64[x] = 8;

// loads
a1 = HEAP8[x];
a2 = HEAP16[x];
a3 = HEAP32[x];
a4 = HEAPU8[x];
a5 = HEAPU16[x];
a6 = HEAPU32[x];
a7 = HEAPF32[x];
a8 = HEAPF64[x];

// store return value
foo = HEAPU8[1337] = 42;

// nesting
HEAP16[bar(HEAPF64[5])];
HEAPF32[x] = HEAP32[y];

// Ignore the special functions themselves. that is, any JS memory access
// will turn into a function call to _asan_js_load_1 etc., which then does
// the memory access for it. It either calls into wasm to get the proper
// asan-instrumented operation, or before the wasm is ready to be called into,
// we must do the access in JS, unsafely. We should not instrument a heap
// access in these functions, as then we'd get infinite recursion - this is
// where we do actually need to still do a HEAP8[..] etc. operation without
// any ASan instrumentation.
function SAFE_HEAP_FOO(ptr) {
  return HEAP8[ptr];
}
function setValue_safe(ptr) {
  return HEAP8[ptr];
}
function getValue_safe(ptr) {
  return HEAP8[ptr];
}

// but do handle everything else
function somethingElse() {
  return HEAP8[ptr];
}

// ignore a.X
HEAP8.length;
HEAP8[length];
