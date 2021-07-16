/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if SAFE_HEAP || !MINIMAL_RUNTIME
// In MINIMAL_RUNTIME, setValue() and getValue() are only available when building with safe heap enabled, for heap safety checking.
// In traditional runtime, setValue() and getValue() are always available (although their use is highly discouraged due to perf penalties)

/** @param {number} ptr
    @param {number} value
    @param {string} type
    @param {number|boolean=} noSafe */
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = {{{ `${POINTER_TYPE}` }}};
#if SAFE_HEAP
  if (noSafe) {
    switch (type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1', undefined, undefined, undefined, '1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8', undefined, undefined, undefined, '1') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16', undefined, undefined, undefined, '1') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32', undefined, undefined, undefined, '1') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64', undefined, undefined, undefined, '1') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float', undefined, undefined, undefined, '1') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double', undefined, undefined, undefined, '1') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
#endif
    switch (type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
}

/** @param {number} ptr
    @param {string} type
    @param {number|boolean=} noSafe */
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = {{{ `${POINTER_TYPE}` }}};
#if SAFE_HEAP
  if (noSafe) {
    switch (type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1', undefined, undefined, undefined, undefined, '1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8', undefined, undefined, undefined, undefined, '1') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16', undefined, undefined, undefined, undefined, '1') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32', undefined, undefined, undefined, undefined, '1') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64', undefined, undefined, undefined, undefined, '1') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float', undefined, undefined, undefined, undefined, '1') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double', undefined, undefined, undefined, undefined, '1') }}};
      default: abort('invalid type for getValue: ' + type);
    }
  } else {
#endif
    switch (type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double') }}};
      default: abort('invalid type for getValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
  return null;
}
#endif // SAFE_HEAP || !MINIMAL_RUNTIME


#if SAFE_HEAP
/** @param {number|boolean=} isFloat */
function getSafeHeapType(bytes, isFloat) {
  switch (bytes) {
    case 1: return 'i8';
    case 2: return 'i16';
    case 4: return isFloat ? 'float' : 'i32';
    case 8: return isFloat ? 'double' : 'i64';
    default: assert(0);
  }
}

#if SAFE_HEAP_LOG
var SAFE_HEAP_COUNTER = 0;
#endif

/** @param {number|boolean=} isFloat */
function SAFE_HEAP_STORE(dest, value, bytes, isFloat) {
#if CAN_ADDRESS_2GB
  dest >>>= 0;
#endif
#if SAFE_HEAP_LOG
  out('SAFE_HEAP store: ' + [dest, value, bytes, isFloat, SAFE_HEAP_COUNTER++]);
#endif
  if (dest <= 0) abort('segmentation fault storing ' + bytes + ' bytes to address ' + dest);
  if (dest % bytes !== 0) abort('alignment error storing to address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (runtimeInitialized) {
    var brk = _sbrk() >>> 0;
    if (dest + bytes > brk) abort('segmentation fault, exceeded the top of the available dynamic heap when storing ' + bytes + ' bytes to address ' + dest + '. DYNAMICTOP=' + brk);
    assert(brk >= _emscripten_stack_get_base()); // sbrk-managed memory must be above the stack
    assert(brk <= HEAP8.length);
  }
  setValue(dest, value, getSafeHeapType(bytes, isFloat), 1);
  return value;
}
function SAFE_HEAP_STORE_D(dest, value, bytes) {
  return SAFE_HEAP_STORE(dest, value, bytes, true);
}

/** @param {number|boolean=} isFloat */
function SAFE_HEAP_LOAD(dest, bytes, unsigned, isFloat) {
#if CAN_ADDRESS_2GB
  dest >>>= 0;
#endif
  if (dest <= 0) abort('segmentation fault loading ' + bytes + ' bytes from address ' + dest);
  if (dest % bytes !== 0) abort('alignment error loading from address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (runtimeInitialized) {
    var brk = _sbrk() >>> 0;
    if (dest + bytes > brk) abort('segmentation fault, exceeded the top of the available dynamic heap when loading ' + bytes + ' bytes from address ' + dest + '. DYNAMICTOP=' + brk);
    assert(brk >= _emscripten_stack_get_base()); // sbrk-managed memory must be above the stack
    assert(brk <= HEAP8.length);
  }
  var type = getSafeHeapType(bytes, isFloat);
  var ret = getValue(dest, type, 1);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1), 10));
#if SAFE_HEAP_LOG
  out('SAFE_HEAP load: ' + [dest, ret, bytes, isFloat, unsigned, SAFE_HEAP_COUNTER++]);
#endif
  return ret;
}
function SAFE_HEAP_LOAD_D(dest, bytes, unsigned) {
  return SAFE_HEAP_LOAD(dest, bytes, unsigned, true);
}

function SAFE_FT_MASK(value, mask) {
  var ret = value & mask;
  if (ret !== value) {
    abort('Function table mask error: function pointer is ' + value + ' which is masked by ' + mask + ', the likely cause of this is that the function pointer is being called by the wrong type.');
  }
  return ret;
}

function segfault() {
  abort('segmentation fault');
}
function alignfault() {
  abort('alignment fault');
}
function ftfault() {
  abort('Function table mask error');
}
#endif

#if USE_ASAN
// C versions of asan_js_{load|store}_* will be used from compiled code, which have
// ASan instrumentation on them. However, until the wasm module is ready, we
// must access things directly.

/** @suppress{duplicate} */
function _asan_js_load_1(ptr) {
  if (runtimeInitialized) return _asan_c_load_1(ptr);
  return HEAP8[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_1u(ptr) {
  if (runtimeInitialized) return _asan_c_load_1u(ptr);
  return HEAPU8[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_2(ptr) {
  if (runtimeInitialized) return _asan_c_load_2(ptr);
  return HEAP16[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_2u(ptr) {
  if (runtimeInitialized) return _asan_c_load_2u(ptr);
  return HEAPU16[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_4(ptr) {
  if (runtimeInitialized) return _asan_c_load_4(ptr);
  return HEAP32[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_4u(ptr) {
  if (runtimeInitialized) return _asan_c_load_4u(ptr) >>> 0;
  return HEAPU32[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_f(ptr) {
  if (runtimeInitialized) return _asan_c_load_f(ptr);
  return HEAPF32[ptr];
}
/** @suppress{duplicate} */
function _asan_js_load_d(ptr) {
  if (runtimeInitialized) return _asan_c_load_d(ptr);
  return HEAPF64[ptr];
}

/** @suppress{duplicate} */
function _asan_js_store_1(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_1(ptr, val);
  return HEAP8[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_1u(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_1u(ptr, val);
  return HEAPU8[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_2(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_2(ptr, val);
  return HEAP16[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_2u(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_2u(ptr, val);
  return HEAPU16[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_4(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_4(ptr, val);
  return HEAP32[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_4u(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_4u(ptr, val) >>> 0;
  return HEAPU32[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_f(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_f(ptr, val);
  return HEAPF32[ptr] = val;
}
/** @suppress{duplicate} */
function _asan_js_store_d(ptr, val) {
  if (runtimeInitialized) return _asan_c_store_d(ptr, val);
  return HEAPF64[ptr] = val;
}
#endif
