function f() {
    var $2 = 0;
    HEAP32[$0 >> 2] = $2 + 1;
    $9 = HEAPU8[$2 >> 0] | 0;
    +HEAPF64[x >> 3];
}

function libraryFunc(ptr, val) {
    if (ptr < HEAP8.length) {
        Atomics.wait(HEAP32, ptr, val);
    }
}
