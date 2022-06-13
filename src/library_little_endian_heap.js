var LibraryLittleEndianHeap = {
  $LE_HEAP_STORE_U16: function(byteOffset, value) {
    HEAP_DATA_VIEW.setUint16(byteOffset, value, true);
  },

  $LE_HEAP_STORE_I16: function(byteOffset, value) {
    HEAP_DATA_VIEW.setInt16(byteOffset, value, true);
  },

  $LE_HEAP_STORE_U32: function(byteOffset, value) {
    HEAP_DATA_VIEW.setUint32(byteOffset, value, true);
  },

  $LE_HEAP_STORE_I32: function(byteOffset, value) {
    HEAP_DATA_VIEW.setInt32(byteOffset, value, true);
  },

  $LE_HEAP_STORE_F32: function(byteOffset, value) {
    HEAP_DATA_VIEW.setFloat32(byteOffset, value, true);
  },

  $LE_HEAP_STORE_F64: function(byteOffset, value) {
    HEAP_DATA_VIEW.setFloat64(byteOffset, value, true);
  },

  $LE_HEAP_LOAD_U16: function(byteOffset) {
    return HEAP_DATA_VIEW.getUint16(byteOffset, true);
  },

  $LE_HEAP_LOAD_I16: function(byteOffset) {
    return HEAP_DATA_VIEW.getInt16(byteOffset, true);
  },

  $LE_HEAP_LOAD_U32: function(byteOffset) {
    return HEAP_DATA_VIEW.getUint32(byteOffset, true);
  },

  $LE_HEAP_LOAD_I32: function(byteOffset) {
    return HEAP_DATA_VIEW.getInt32(byteOffset, true);
  },

  $LE_HEAP_LOAD_F32: function(byteOffset) {
    return HEAP_DATA_VIEW.getFloat32(byteOffset, true);
  },

  $LE_HEAP_LOAD_F64: function(byteOffset) {
    return HEAP_DATA_VIEW.getFloat64(byteOffset, true);
  }
}

mergeInto(LibraryManager.library, LibraryLittleEndianHeap);
