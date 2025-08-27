a = (growMemViews(), HEAP8)[SAFE_HEAP_INDEX((growMemViews(), HEAP8), x, "loading")];

(growMemViews(), HEAP8)[SAFE_HEAP_INDEX((growMemViews(), HEAP8), x, "storing")] = a;

a = (growMemViews(), HEAPU8)[SAFE_HEAP_INDEX((growMemViews(), HEAPU8), x, "loading")];

(growMemViews(), HEAPU8)[SAFE_HEAP_INDEX((growMemViews(), HEAPU8), x, "storing")] = a;

a = LE_HEAP_LOAD_I16((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAP16), x, "loading") * 2));

LE_HEAP_STORE_I16((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAP16), x, "storing") * 2), a);

a = LE_HEAP_LOAD_U16((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPU16), x, "loading") * 2));

LE_HEAP_STORE_U16((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPU16), x, "storing") * 2), a);

a = LE_HEAP_LOAD_I32((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAP32), x, "loading") * 4));

LE_HEAP_STORE_I32((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAP32), x, "storing") * 4), a);

a = LE_HEAP_LOAD_U32((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPU32), x, "loading") * 4));

LE_HEAP_STORE_U32((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPU32), x, "storing") * 4), a);

a = LE_HEAP_LOAD_F32((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPF32), x, "loading") * 4));

LE_HEAP_STORE_F32((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPF32), x, "storing") * 4), a);

a = LE_HEAP_LOAD_F64((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPF64), x, "loading") * 8));

LE_HEAP_STORE_F64((growMemViews(), SAFE_HEAP_INDEX((growMemViews(), HEAPF64), x, "storing") * 8), a);

HEAP[x];

HeAp[x];

LE_ATOMICS_ADD(heap, offset, value);

LE_ATOMICS_AND(heap, offset, value);

LE_ATOMICS_COMPAREEXCHANGE(heap, offset, expected, replacement);

LE_ATOMICS_EXCHANGE(heap, offset, value);

LE_ATOMICS_LOAD(heap, offset);

LE_ATOMICS_OR(heap, offset, value);

LE_ATOMICS_SUB(heap, offset, value);

LE_ATOMICS_WAIT(heap, offset, value, timeout);

LE_ATOMICS_WAITASYNC(heap, offset, value, timeout);

LE_ATOMICS_XOR(heap, offset, value);
