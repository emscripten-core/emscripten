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

// something completely different
process.versions.node;

insideCall(HEAP32[x >> 2]);

heap[x];

HEAP[x];

// but not this
HeAp[x];

HEAP64[x >> 3] = HEAPU64[y];
