a = HEAP8[x];     // HEAP8
HEAP8[x] = a;
a = HEAPU8[x];    // HEAPU8
HEAPU8[x] = a;
a = HEAP16[x];    // HEAP16
HEAP16[x] = a;
a = HEAPU16[x];   // HEAPU16
HEAPU16[x] = a;
a = HEAP32[x];    // HEAPI32
HEAP32[x] = a;
a = HEAPU32[x];   // HEAPU32
HEAPU32[x] = a;
a = HEAPF32[x];   // HEAPF32
HEAPF32[x] = a;
a = HEAPF64[x];   // HEAPF64
HEAPF64[x] = a;
HEAP[x];          // should not be changed
HeAp[x];
Atomics.add(heap, offset, value);
Atomics.and(heap, offset, value);
Atomics.compareExchange(heap, offset, expected, replacement);
Atomics.exchange(heap, offset, value);
Atomics.load(heap, offset);
Atomics.or(heap, offset, value);
Atomics.sub(heap, offset, value);
Atomics.wait(heap, offset, value, timeout);
Atomics.waitAsync(heap, offset, value, timeout);
Atomics.xor(heap, offset, value);
