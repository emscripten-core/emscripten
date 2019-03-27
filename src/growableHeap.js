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

