function f() {
  var $2 = 0;
  GROWABLE_HEAP(HEAP32)[$0 >> 2] = $2 + 1;
  $9 = GROWABLE_HEAP(HEAPU8)[$2 >> 0] | 0;
  +GROWABLE_HEAP(HEAPF64)[x >> 3];
  GROWABLE_HEAP(HEAP64)[x >> 3] = GROWABLE_HEAP(HEAP64)[y >> 3];
}

function libraryFunc(ptr, val) {
  if (ptr < GROWABLE_HEAP(HEAP8).length) {
    Atomics.wait(GROWABLE_HEAP(HEAP32), ptr, val);
  }
}
