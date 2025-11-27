var LibraryLittleEndianHeap = {
  $LE_HEAP_STORE_U16: (byteOffset, value) =>
    HEAP_DATA_VIEW.setUint16(byteOffset, value, true),

  $LE_HEAP_STORE_I16: (byteOffset, value) =>
    HEAP_DATA_VIEW.setInt16(byteOffset, value, true),

  $LE_HEAP_STORE_U32: (byteOffset, value) =>
    HEAP_DATA_VIEW.setUint32(byteOffset, value, true),

  $LE_HEAP_STORE_I32: (byteOffset, value) =>
    HEAP_DATA_VIEW.setInt32(byteOffset, value, true),

  $LE_HEAP_STORE_U64: (byteOffset, value) =>
    HEAP_DATA_VIEW.setBigUint64(byteOffset, value, true),

  $LE_HEAP_STORE_I64: (byteOffset, value) =>
    HEAP_DATA_VIEW.setBigInt64(byteOffset, value, true),

  $LE_HEAP_STORE_F32: (byteOffset, value) =>
    HEAP_DATA_VIEW.setFloat32(byteOffset, value, true),

  $LE_HEAP_STORE_F64: (byteOffset, value) =>
    HEAP_DATA_VIEW.setFloat64(byteOffset, value, true),

  $LE_HEAP_LOAD_U16: (byteOffset) =>
    HEAP_DATA_VIEW.getUint16(byteOffset, true),

  $LE_HEAP_LOAD_I16: (byteOffset) =>
    HEAP_DATA_VIEW.getInt16(byteOffset, true),

  $LE_HEAP_LOAD_U32: (byteOffset) =>
    HEAP_DATA_VIEW.getUint32(byteOffset, true),

  $LE_HEAP_LOAD_I32: (byteOffset) =>
    HEAP_DATA_VIEW.getInt32(byteOffset, true),

  $LE_HEAP_LOAD_U64: (byteOffset) =>
    HEAP_DATA_VIEW.getBigUint64(byteOffset, true),

  $LE_HEAP_LOAD_I64: (byteOffset) =>
    HEAP_DATA_VIEW.getBigInt64(byteOffset, true),

  $LE_HEAP_LOAD_F32: (byteOffset) =>
    HEAP_DATA_VIEW.getFloat32(byteOffset, true),

  $LE_HEAP_LOAD_F64: (byteOffset) =>
    HEAP_DATA_VIEW.getFloat64(byteOffset, true),

  $LE_ATOMICS_NATIVE_BYTE_ORDER__postset: `
LE_ATOMICS_NATIVE_BYTE_ORDER = (new Int8Array(new Int16Array([1]).buffer)[0] === 1)
  ? [ /* little endian */
    (x => x),
    (x => x),
    undefined,
    (x => x),
  ]
  : [ /* big endian */
    (x => x),
    (x => (((x & 0xff00) << 8) | ((x & 0xff) << 24)) >> 16),
    undefined,
    (x => ((x >> 24) & 0xff) | ((x >> 8) & 0xff00) | ((x & 0xff00) << 8) | ((x & 0xff) << 24)),
  ];
function LE_HEAP_UPDATE() {
  HEAPU16.unsigned = (x => x & 0xffff);
  HEAPU32.unsigned = (x => x >>> 0);
}
  `,
  $LE_ATOMICS_NATIVE_BYTE_ORDER: [],

  $LE_ATOMICS_ADD: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.add(heap, offset, order(value)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_AND: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.and(heap, offset, order(value)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_COMPAREEXCHANGE: (heap, offset, expected, replacement) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.compareExchange(heap, offset, order(expected), order(replacement)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_EXCHANGE: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.exchange(heap, offset, order(value)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_ISLOCKFREE: (size) => Atomics.isLockFree(size),
  $LE_ATOMICS_LOAD: (heap, offset) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.load(heap, offset));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_NOTIFY: (heap, offset, count) => Atomics.notify(heap, offset, count),
  $LE_ATOMICS_OR: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.or(heap, offset, order(value)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_STORE: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    Atomics.store(heap, offset, order(value));
  },
  $LE_ATOMICS_SUB: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.sub(heap, offset, order(value)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
  $LE_ATOMICS_WAIT: (heap, offset, value, timeout = Infinity) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    return Atomics.wait(heap, offset, order(value), timeout);
  },
  $LE_ATOMICS_WAITASYNC: (heap, offset, value, timeout = Infinity) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    return Atomics.waitAsync(heap, offset, order(value), timeout);
  },
  $LE_ATOMICS_XOR: (heap, offset, value) => {
    const order = LE_ATOMICS_NATIVE_BYTE_ORDER[heap.BYTES_PER_ELEMENT - 1];
    const res = order(Atomics.xor(heap, offset, order(value)));
    return heap.unsigned ? heap.unsigned(res) : res;
  },
}

addToLibrary(LibraryLittleEndianHeap);
