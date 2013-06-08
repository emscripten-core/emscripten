function a() {
  f((HEAPU8[10202] | 0) + 5 | 0);
  f((HEAPU8[10202] | 0) | 0);
  f(347 | 0);
  f(347 | 12);
  f(347 & 12);
  HEAP[4096 >> 2] = 5;
  HEAP[(4096 & 8191) >> 2] = 5;
  whee(12, 13) | 0;
  +whee(12, 13);
  f((g = t(), (g+g)|0)|0);
}
function b($this, $__n) {
  $this = $this | 0;
  $__n = $__n | 0;
  var $4 = 0, $5 = 0, $10 = 0, $13 = 0, $14 = 0, $15 = 0, $23 = 0, $30 = 0, $38 = 0, $40 = 0;
  if (($__n | 0) == 0) {
    return;
  }
  $4 = $this;
  $5 = HEAP8[$4 & 16777215] | 0;
  if (($5 & 1) << 24 >> 24 == 0) {
    $14 = 10;
    $13 = $5;
  } else {
    $10 = HEAP32[(($this | 0) & 16777215) >> 2] | 0;
    $14 = ($10 & -2) - 1 | 0;
    $13 = $10 & 255;
  }
  $15 = $13 & 255;
  if (($15 & 1 | 0) == 0) {
    $23 = $15 >>> 1;
  } else {
    $23 = HEAP32[(($this + 4 | 0) & 16777215) >> 2] | 0;
  }
  if (($14 - $23 | 0) >>> 0 < $__n >>> 0) {
    __ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE9__grow_byEjjjjjj($this, $14, ($__n - $14 | 0) + $23 | 0, $23, $23);
    $30 = HEAP8[$4 & 16777215] | 0;
  } else {
    $30 = $13;
  }
  if (($30 & 1) << 24 >> 24 == 0) {
    $38 = $this + 1 | 0;
  } else {
    $38 = HEAP32[(($this + 8 | 0) & 16777215) >> 2] | 0;
  }
  _memset($38 + $23 | 0 | 0 | 0, 0 | 0 | 0, $__n | 0 | 0, 1 | 0 | 0, 1213141516);
  $40 = $23 + $__n | 0;
  if (((HEAP8[$4 & 16777215] | 0) & 1) << 24 >> 24 == 0) {
    HEAP8[$4 & 16777215] = $40 << 1 & 255;
  } else {
    HEAP32[(($this + 4 | 0) & 16777215) >> 2] = $40;
  }
  HEAP8[($38 + $40 | 0) & 16777215] = 0;
  return;
}
function rett() {
  if (f()) {
    g();
    return 5;
  }
  // missing final return, need to add it
}
function ret2t() {
  if (f()) {
    g();
    return;
  }
  // missing final return, but no need
}
function retf() {
  if (f()) {
    g();
    return +h();
  }
  // missing final return, need it as a float
}
function i32_8() {
  if (((HEAP8[$4 & 16777215] | 0) << 24 >> 24) == 0) {
    print(5);
  }
  if ((HEAP8[$5 & 16777215] << 24 >> 24) == 0) {
    print(5);
  }
  if (((HEAPU8[$6 & 16777215] | 0) << 24 >> 24) == 0) {
    print(5);
  }
  if ((HEAPU8[$7 & 16777215] << 24 >> 24) == 0) {
    print(5);
  }
  // non-valid
  if ((HEAPU8[$8 & 16777215] << 24 >> 16) == 0) {
    print(5);
  }
  if ((HEAPU8[$9 & 16777215] << 16 >> 16) == 0) {
    print(5);
  }
}
function sign_extension_simplification() {
  if ((HEAP8[$4 & 16777215] & 127) << 24 >> 24 == 0) {
    print(5);
  }
  if ((HEAP8[$4 & 16777215] & 128) << 24 >> 24 == 0) {
    print(5);
  }
  if ((HEAP32[$5 & 16777215] & 32767) << 16 >> 16 == 0) {
    print(5);
  }
  if ((HEAP32[$5 & 16777215] & 32768) << 16 >> 16 == 0) {
    print(5);
  }
}
function tempDoublePtr($45, $14, $28, $42) {
  $45 = $45 | 0;
  $14 = $14 | 0;
  $28 = $28 | 0;
  $42 = $42 | 0;
  var unelim = 0; // only used as assign to int heap, so can avoid bitcast in definition
  var bad = 0;
  var unelim2 = 0; // opposite types
  unelim = (HEAPF32[tempDoublePtr >> 2] = 127.5 * +$14, HEAP32[tempDoublePtr >> 2] | 0);
  HEAP32[$45 >> 2] = 0 | (HEAPF32[tempDoublePtr >> 2] = ($14 < $28 ? $14 : $28) - $42, HEAP32[tempDoublePtr >> 2] | 0);
  HEAP32[$world + 102916 >> 2] = _malloc(192) | 0;
  f((HEAP32[tempDoublePtr >> 2] = HEAP32[$45 >> 2], +HEAPF32[tempDoublePtr >> 2]));
  g((HEAPF32[tempDoublePtr >> 2] = HEAPF32[$14 >> 2], HEAP32[tempDoublePtr >> 2] | 0));
  $42 = (HEAP32[tempDoublePtr >> 2] = HEAP32[$42 >> 2] | 0, +HEAPF32[tempDoublePtr >> 2]);
  ch($42);
  HEAP32[$45 >> 2] = unelim;
  moar();
  bad = (HEAPF32[tempDoublePtr >> 2] = 127.5 * +$14, HEAP32[tempDoublePtr >> 2] | 0);
  func();
  HEAP32[4] = bad;
  HEAP32[5] = (bad + 1) | 0;
  moar();
  unelim2 = (HEAP32[tempDoublePtr >> 2] = 127 + $14, +HEAPF32[tempDoublePtr >> 2]);
  func();
  HEAPF32[4] = unelim2;
}
function boxx($this, $aabb, $xf, $childIndex) {
  $this = $this | 0;
  $aabb = $aabb | 0;
  $xf = $xf | 0;
  $childIndex = $childIndex | 0;
  var $2 = +0, $4 = +0, $7 = +0, $9 = +0, $13 = +0, $14 = +0, $19 = +0, $20 = +0, $22 = +0, $25 = +0, $28 = +0, $32 = +0, $42 = +0, $45 = 0, $_sroa_06_0_insert_insert$1 = 0, $51 = 0, $_sroa_0_0_insert_insert$1 = 0;
  $2 = +HEAPF32[$xf + 12 >> 2];
  $4 = +HEAPF32[$this + 12 >> 2];
  $7 = +HEAPF32[$xf + 8 >> 2];
  $9 = +HEAPF32[$this + 16 >> 2];
  $13 = +HEAPF32[$xf >> 2];
  $14 = $13 + ($2 * $4 - $7 * $9);
  $19 = +HEAPF32[$xf + 4 >> 2];
  $20 = $4 * $7 + $2 * $9 + $19;
  $22 = +HEAPF32[$this + 20 >> 2];
  $25 = +HEAPF32[$this + 24 >> 2];
  $28 = $13 + ($2 * $22 - $7 * $25);
  $32 = $19 + ($7 * $22 + $2 * $25);
  $42 = +HEAPF32[$this + 8 >> 2];
  $45 = $aabb;
  $_sroa_06_0_insert_insert$1 = (HEAPF32[tempDoublePtr >> 2] = ($20 < $32 ? $20 : $32) - $42, HEAP32[tempDoublePtr >> 2] | 0) | 0;
  HEAPF32[$45 >> 2] = ($14 < $28 ? $14 : $28) - $42;
  HEAP32[$45 + 4 >> 2] = $_sroa_06_0_insert_insert$1;
  $51 = $aabb + 8 | 0;
  $_sroa_0_0_insert_insert$1 = (HEAPF32[tempDoublePtr >> 2] = $42 + ($20 > $32 ? $20 : $32), HEAP32[tempDoublePtr >> 2] | 0) | 0;
  HEAPF32[$51 >> 2] = $42 + ($14 > $28 ? $14 : $28);
  HEAP32[$51 + 4 >> 2] = $_sroa_0_0_insert_insert$1;
  return;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b", "rett", "ret2t", "retf", "i32_8", "tempDoublePtr", "boxx"]
