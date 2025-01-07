a = HEAP8[x];

HEAP8[x] = a;

a = HEAPU8[x];

HEAPU8[x] = a;

a = LE_HEAP_LOAD_I16(x * 2);

LE_HEAP_STORE_I16(x * 2, a);

a = LE_HEAP_LOAD_U16(x * 2);

LE_HEAP_STORE_U16(x * 2, a);

a = LE_HEAP_LOAD_I32(x * 4);

LE_HEAP_STORE_I32(x * 4, a);

a = LE_HEAP_LOAD_U32(x * 4);

LE_HEAP_STORE_U32(x * 4, a);

a = LE_HEAP_LOAD_F32(x * 4);

LE_HEAP_STORE_F32(x * 4, a);

a = LE_HEAP_LOAD_F64(x * 8);

LE_HEAP_STORE_F64(x * 8, a);

HEAP[x];

HeAp[x];
