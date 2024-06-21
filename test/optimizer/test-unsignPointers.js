/**
 * This is a multi-line comment
 */
HEAP32[x >> 2];

// This is a single-line comment
HEAP8[x];

HEAP8.length;

HEAP16.set(foo);

HEAP16.set(foo, x);

HEAPU32.copyWithin(x, y);

HEAPU32.copyWithin(x, y, z);

HEAPU8.subarray();

HEAPU8.subarray(x);

HEAPU8.subarray(x, y);

process.versions.node; // something completely different

insideCall(HEAP32[x >> 2]);

heap[x];

HEAP[x];

HeAp[x]; // but not this
