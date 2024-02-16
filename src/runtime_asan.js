/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !USE_ASAN
#error "should only be inclded in USE_ASAN mode"
#endif

// C versions of asan_js_{load|store}_* will be used from compiled code, which have
// ASan instrumentation on them. However, until the wasm module is ready, we
// must access things directly.

/** @suppress{duplicate} */
function _asan_js_load_1(ptr) {
  if (runtimeInitialized) return __asan_c_load_1(ptr);
  return HEAP8[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_1u(ptr) {
  if (runtimeInitialized) return __asan_c_load_1u(ptr);
  return HEAPU8[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_2(ptr) {
  if (runtimeInitialized) return __asan_c_load_2(ptr);
  return HEAP16[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_2u(ptr) {
  if (runtimeInitialized) return __asan_c_load_2u(ptr);
  return HEAPU16[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_4(ptr) {
  if (runtimeInitialized) return __asan_c_load_4(ptr);
  return HEAP32[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_4u(ptr) {
  if (runtimeInitialized) return __asan_c_load_4u(ptr) >>> 0;
  return HEAPU32[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_f(ptr) {
  if (runtimeInitialized) return __asan_c_load_f(ptr);
  return HEAPF32[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_d(ptr) {
  if (runtimeInitialized) return __asan_c_load_d(ptr);
  return HEAPF64[ptr];
}

/** @suppress{duplicate} */
function _asan_js_store_1(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_1(ptr, val);
  return HEAP8[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_1u(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_1u(ptr, val);
  return HEAPU8[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_2(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_2(ptr, val);
  return HEAP16[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_2u(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_2u(ptr, val);
  return HEAPU16[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_4(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_4(ptr, val);
  return HEAP32[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_4u(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_4u(ptr, val) >>> 0;
  return HEAPU32[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_f(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_f(ptr, val);
  return HEAPF32[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_d(ptr, val) {
  if (runtimeInitialized) return __asan_c_store_d(ptr, val);
  return HEAPF64[ptr] = val;
}
