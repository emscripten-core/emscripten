// Support for growable heap + pthreads, where the buffer may change, so JS views
// must be updated.
function GROWABLE_HEAP_STORE_I8(ptr, value) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  HEAP8[ptr] = value;
}
function GROWABLE_HEAP_STORE_I16(ptr, value) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  HEAP16[ptr >> 1] = value;
}
function GROWABLE_HEAP_STORE_I32(ptr, value) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  HEAP32[ptr >> 2] = value;
}
function GROWABLE_HEAP_STORE_F32(ptr, value) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  HEAPF32[ptr >> 2] = value;
}
function GROWABLE_HEAP_STORE_F64(ptr, value) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  HEAPF64[ptr >> 3] = value;
}

function GROWABLE_HEAP_LOAD_I8(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAP8[ptr];
}
function GROWABLE_HEAP_LOAD_U8(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAPU8[ptr];
}
function GROWABLE_HEAP_LOAD_I16(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAP16[ptr >> 1];
}
function GROWABLE_HEAP_LOAD_U16(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAPU16[ptr >> 1];
}
function GROWABLE_HEAP_LOAD_I32(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAP32[ptr >> 2];
}
function GROWABLE_HEAP_LOAD_U32(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAPU32[ptr >> 2];
}
function GROWABLE_HEAP_LOAD_F32(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAPF32[ptr >> 2];
}
function GROWABLE_HEAP_LOAD_F64(ptr) {
  if (wasmMemory.buffer != buffer) {
    buffer = wasmMemory.buffer;
    updateGlobalBufferViews();
  }
  return HEAPF64[ptr >> 3];
}
function GROWABLE_HEAP_VIEW_I8(start, end) {
  return new Int8Array(wasmMemory.buffer, start, end - start);
}
function GROWABLE_HEAP_VIEW_U8(start, end) {
  return new Uint8Array(wasmMemory.buffer, start, end - start);
}
function GROWABLE_HEAP_VIEW_I16(start, end) {
  return new Int16Array(wasmMemory.buffer, start >> 1, end - start >> 1);
}
function GROWABLE_HEAP_VIEW_U16(start, end) {
  return new Uint16Array(wasmMemory.buffer, start >> 1, end - start >> 1);
}
function GROWABLE_HEAP_VIEW_I32(start, end) {
  return new Int32Array(wasmMemory.buffer, start >> 2, end - start >> 2);
}
function GROWABLE_HEAP_VIEW_U32(start, end) {
  return new Uint32Array(wasmMemory.buffer, start >> 2, end - start >> 2);
}
function GROWABLE_HEAP_VIEW_F32(start, end) {
  return new Float32Array(wasmMemory.buffer, start >> 2, end - start >> 2);
}
function GROWABLE_HEAP_VIEW_F64(start, end) {
  return new Float64Array(wasmMemory.buffer, start >> 3, end - start >> 3);
}

