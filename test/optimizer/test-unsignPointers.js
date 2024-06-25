/**
 * Multi-line comment.
 */
/**
 * Another multi-line comment.
 */
/**
 * A third multi-line comment.
 */
HEAP32[x >> 2];

// Single line comment
// Another single line comment
// A third single line comment
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
