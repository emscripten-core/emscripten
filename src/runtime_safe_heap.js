/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !SAFE_HEAP
#error "should only be included in SAFE_HEAP mode"
#endif

#if SAFE_HEAP_LOG
var SAFE_HEAP_COUNTER = 0;
#endif

function SAFE_HEAP_INDEX(arr, idx, action) {
#if CAN_ADDRESS_2GB
  idx >>>= 0;
#endif
  const bytes = arr.BYTES_PER_ELEMENT;
  const dest = idx * bytes;
#if SAFE_HEAP_LOG
  dbg(`SAFE_HEAP ${action}: ${[arr.constructor.name, idx, SAFE_HEAP_COUNTER++]}`);
#endif
  if (idx <= 0) abort(`segmentation fault ${action} ${bytes} bytes at address ${dest}`);
#if EXIT_RUNTIME
  if (runtimeInitialized && !runtimeExited) {
#else
  if (runtimeInitialized) {
#endif
    var brk = _sbrk(0);
    if (dest + bytes > brk) abort(`segmentation fault, exceeded the top of the available dynamic heap when ${action} ${bytes} bytes at address ${dest}. DYNAMICTOP=${brk}`);
    if (brk < _emscripten_stack_get_base()) abort(`brk >= _emscripten_stack_get_base() (brk=${brk}, _emscripten_stack_get_base()=${_emscripten_stack_get_base()})`); // sbrk-managed memory must be above the stack
    if (brk > wasmMemory.buffer.byteLength) abort(`brk <= wasmMemory.buffer.byteLength (brk=${brk}, wasmMemory.buffer.byteLength=${wasmMemory.buffer.byteLength})`);
  }
  return idx;
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
