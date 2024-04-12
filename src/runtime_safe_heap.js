/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !SAFE_HEAP
#error "should only be inclded in SAFE_HEAP mode"
#endif

/** @param {number|boolean=} isFloat */
function getSafeHeapType(bytes, isFloat) {
  switch (bytes) {
    case 1: return 'i8';
    case 2: return 'i16';
    case 4: return isFloat ? 'float' : 'i32';
    case 8: return isFloat ? 'double' : 'i64';
    default: abort(`getSafeHeapType() invalid bytes=${bytes}`);
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
  dbg('SAFE_HEAP store: ' + [dest, value, bytes, isFloat, SAFE_HEAP_COUNTER++]);
#endif
  if (dest <= 0) abort(`segmentation fault storing ${bytes} bytes to address ${dest}`);
#if SAFE_HEAP == 1
  if (dest % bytes !== 0) abort(`alignment error storing to address ${dest}, which was expected to be aligned to a multiple of ${bytes}`);
#else
  if (dest % bytes !== 0) warnOnce(`alignment error in a memory store operation, alignment was a multiple of ${(((dest ^ (dest-1)) >> 1) + 1)}, but was was expected to be aligned to a multiple of ${bytes}`);
#endif
#if EXIT_RUNTIME
  if (runtimeInitialized && !runtimeExited) {
#else
  if (runtimeInitialized) {
#endif
    var brk = _sbrk(0);
    if (dest + bytes > brk) abort(`segmentation fault, exceeded the top of the available dynamic heap when storing ${bytes} bytes to address ${dest}. DYNAMICTOP=${brk}`);
    if (brk < _emscripten_stack_get_base()) abort(`brk >= _emscripten_stack_get_base() (brk=${brk}, _emscripten_stack_get_base()=${_emscripten_stack_get_base()})`); // sbrk-managed memory must be above the stack
    if (brk > wasmMemory.buffer.byteLength) abort(`brk <= wasmMemory.buffer.byteLength (brk=${brk}, wasmMemory.buffer.byteLength=${wasmMemory.buffer.byteLength})`);
  }
  setValue_safe(dest, value, getSafeHeapType(bytes, isFloat));
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
  if (dest <= 0) abort(`segmentation fault loading ${bytes} bytes from address ${dest}`);
#if SAFE_HEAP == 1
  if (dest % bytes !== 0) abort(`alignment error loading from address ${dest}, which was expected to be aligned to a multiple of ${bytes}`);
#else
  if (dest % bytes !== 0) warnOnce(`alignment error in a memory load operation, alignment was a multiple of ${(((dest ^ (dest-1)) >> 1) + 1)}, but was was expected to be aligned to a multiple of ${bytes}`);
#endif
#if EXIT_RUNTIME
  if (runtimeInitialized && !runtimeExited) {
#else
  if (runtimeInitialized) {
#endif
    var brk = _sbrk(0);
    if (dest + bytes > brk) abort(`segmentation fault, exceeded the top of the available dynamic heap when loading ${bytes} bytes from address ${dest}. DYNAMICTOP=${brk}`);
    if (brk < _emscripten_stack_get_base()) abort(`brk >= _emscripten_stack_get_base() (brk=${brk}, _emscripten_stack_get_base()=${_emscripten_stack_get_base()})`); // sbrk-managed memory must be above the stack
    if (brk > wasmMemory.buffer.byteLength) abort(`brk <= wasmMemory.buffer.byteLength (brk=${brk}, wasmMemory.buffer.byteLength=${wasmMemory.buffer.byteLength})`);
  }
  var type = getSafeHeapType(bytes, isFloat);
  var ret = getValue_safe(dest, type);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1), 10));
#if SAFE_HEAP_LOG
  dbg('SAFE_HEAP load: ' + [dest, ret, bytes, isFloat, unsigned, SAFE_HEAP_COUNTER++]);
#endif
  return ret;
}
function SAFE_HEAP_LOAD_D(dest, bytes, unsigned) {
  return SAFE_HEAP_LOAD(dest, bytes, unsigned, true);
}

function SAFE_FT_MASK(value, mask) {
  var ret = value & mask;
  if (ret !== value) {
    abort(`Function table mask error: function pointer is ${value} which is masked by ${mask}, the likely cause of this is that the function pointer is being called by the wrong type.`);
  }
  return ret;
}

function segfault() {
  abort('segmentation fault');
}
function alignfault() {
#if SAFE_HEAP == 1
  abort('alignment fault');
#else
  warnOnce('alignment fault');
#endif
}
