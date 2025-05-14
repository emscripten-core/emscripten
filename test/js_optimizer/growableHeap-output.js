function f() {
  var $2 = 0;
  (maybeUpdateMemoryViews(), HEAP32)[$0 >> 2] = $2 + 1;
  $9 = (maybeUpdateMemoryViews(), HEAPU8)[$2 >> 0] | 0;
  +(maybeUpdateMemoryViews(), HEAPF64)[x >> 3];
  (maybeUpdateMemoryViews(), HEAP64)[x >> 3] = (maybeUpdateMemoryViews(), HEAP64)[y >> 3];
}

function libraryFunc(ptr, val) {
  if (ptr < (maybeUpdateMemoryViews(), HEAP8).length) {
    Atomics.wait((maybeUpdateMemoryViews(), HEAP32), ptr, val);
  }
}
