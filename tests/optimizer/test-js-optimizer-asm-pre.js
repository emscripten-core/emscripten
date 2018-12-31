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
 // always coerce function calls in asm
 f() | 0;
 f((h() | 0) + 5 | 0);
 f(((x + y) | 0) + z | 0);
 +f();
 f(+(+h() + 5));
 $140 = $p_3_i + (-$mantSize_0_i | 0) | 0;
 f(g() | 0 | 0);
 f(g() | 0 & -1);
 f((g() | 0) >> 2);
 $56 = (_fcntl() | 0) | 1;
 FUNCTION_TABLE_ii[55 & 127]() | 0;
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
 // Eliminate the |0.
 HEAP32[$4] = ((~(HEAP32[$5]|0))|0);
 // Eliminate the &255
 HEAP8[$4] = HEAP32[$5]&255;
 // Eliminate the &65535
 HEAP16[$4] = HEAP32[$5]&65535;
 // Rewrite to ~.
 HEAP32[$4] = HEAP32[$5]^-1;
 // Rewrite to ~ and eliminate the |0.
 HEAP32[$4] = ((HEAP32[$5]|0)^-1)|0;
 h((~~g) ^ -1); // do NOT convert this, as it would lead to ~~~ which is confusing in asm, given the role of ~~
 return;
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
 if ((HEAP32[$5 & 16777215] << 23 >> 23 & 254) == 0) {
  print(5);
 }
 if ((HEAP32[$5 & 16777215] << 23 >> 23 & 255) == 0) {
  print(5);
 }
 if ((HEAP32[$5 & 16777215] << 24 >> 24 & 255) == 0) {
  print(5);
 }
 if ((HEAP32[$5 & 16777215] << 25 >> 25 & 255) == 0) {
  print(5);
 }
 HEAP16[$6 >> 1] = (($1 << 24 >> 24 & 65535) * 28335 | 0) & 65535;
}
function compare_result_simplification() {
 f(((a > b)&1) + 1 | 0);
 f(((a > b)&1) | z);
 f(((a > b)&1) | (c > d & 1));
 // Don't eliminate these '&1's.
 HEAP32[$4] = (HEAP32[$5] < HEAP32[$6]) & 1;
 var x = (HEAP32[$5] != HEAP32[$6]) & 1;
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
 barrier();
 $f163 = (HEAP32[tempDoublePtr >> 2] = HEAP32[$f165 >> 2], HEAP32[tempDoublePtr + 4 >> 2] = HEAP32[$f165 + 4 >> 2], +HEAPF64[tempDoublePtr >> 3]);
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
function _main($argc, $argv) {
 $argc = $argc | 0;
 $argv = $argv | 0;
 var $def_i21 = 0, $def_i = 0, $world = 0, $bd = 0, $shape = 0, $shape1 = 0, $bd2 = 0, $result = 0, $6 = 0, $WARMUP_0 = 0, $14 = 0, $15 = 0, $17 = 0, $i_09_i_i = 0, $j_08_i_i = 0, $34 = 0, $j_1_i_i = 0, $38 = 0, $46 = 0, $48 = 0, $50 = 0, $54 = 0, $i_05_i_i_i = 0, $56 = 0, $62 = 0, $_lcssa_i_i_i = 0, $87 = 0, $96 = 0, $97 = 0, $98 = 0, $112 = 0, $115 = 0, $116 = 0, $118 = 0, $121 = 0, $126 = 0, $135 = 0, $137 = 0, $174 = 0, $176 = 0, $177 = 0, $178 = 0, $179 = 0, $180 = 0, $181 = 0, $182 = 0, $183 = 0, $185 = 0, $186 = 0, $188 = 0, $189 = 0, $190 = 0, $191 = 0, $192 = 0, $193 = 0, $194 = 0, $195 = 0, $196 = 0, $i_057 = 0, $x_sroa_0_0_load303656 = +0, $x_sroa_1_4_load313755 = +0, $j_052 = 0, $y_sroa_0_0_load283451 = +0, $y_sroa_1_4_load293550 = +0, $y_sroa_0_0_insert_insert$1 = 0, $205 = 0, $208 = 0, $209 = 0, $213 = 0, $223 = 0, $236 = 0, $i3_042 = 0, $241 = 0, $242 = 0, $243 = 0, $i4_038 = 0, $245 = 0, $260 = +0, $_0 = 0, label = 0, __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 103416 | 0;
 $def_i21 = __stackBase__ | 0;
 $def_i = __stackBase__ + 32 | 0;
 $world = __stackBase__ + 64 | 0;
 $bd = __stackBase__ + 103096 | 0;
 $shape = __stackBase__ + 103152 | 0;
 $shape1 = __stackBase__ + 103200 | 0;
 $bd2 = __stackBase__ + 103352 | 0;
 $result = __stackBase__ + 103408 | 0;
 do {
  if (($argc | 0) > 1) {
   $6 = (HEAP8[HEAP32[($argv + 4 | 0) >> 2] | 0] | 0) << 24 >> 24;
   if (($6 | 0 | 0) == (49 | 0)) {
    HEAP32[9656 >> 2] = 35;
    $WARMUP_0 = 5;
    break;
   } else if (($6 | 0 | 0) == (50 | 0)) {
    HEAP32[9656 >> 2] = 161;
    $WARMUP_0 = 32;
    break;
   } else if (($6 | 0 | 0) == (51 | 0)) {
    label = 43;
    break;
   } else if (($6 | 0 | 0) == (52 | 0)) {
    HEAP32[9656 >> 2] = 2331;
    $WARMUP_0 = 320;
    break;
   } else if (($6 | 0 | 0) == (53 | 0)) {
    HEAP32[9656 >> 2] = 5661;
    $WARMUP_0 = 640;
    break;
   } else if (($6 | 0 | 0) == (48 | 0)) {
    $_0 = 0;
    STACKTOP = __stackBase__;
    return $_0 | 0;
   } else {
    _printf(3512 | 0 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[tempInt >> 2] = $6 - 48 | 0, tempInt) | 0) | 0;
    $_0 = -1;
    STACKTOP = __stackBase__;
    return $_0 | 0;
   }
  } else {
   label = 43;
  }
 } while (0);
 if ((label | 0) == 43) {
  HEAP32[9656 >> 2] = 333;
  $WARMUP_0 = 64;
 }
 $14 = $world | 0;
 $15 = $world + 8 | 0;
 HEAP32[$15 >> 2] = 128;
 HEAP32[($world + 4 | 0) >> 2] = 0;
 $17 = _malloc(1024) | 0;
 HEAP32[($world | 0) >> 2] = $17;
 _memset($17 | 0 | 0, 0 | 0 | 0, (HEAP32[$15 >> 2] | 0) << 3 | 0 | 0);
 _memset($world + 12 | 0 | 0 | 0, 0 | 0 | 0, 56 | 0 | 0);
 $j_08_i_i = 0;
 $i_09_i_i = 1;
 while (1) {
  if (!(($j_08_i_i | 0) < 14)) {
   label = 49;
   break;
  }
  if (($i_09_i_i | 0) > (HEAP32[(9600 + ($j_08_i_i << 2) | 0) >> 2] | 0 | 0)) {
   $34 = $j_08_i_i + 1 | 0;
   HEAP8[$i_09_i_i + 8952 | 0] = $34 & 255;
   $j_1_i_i = $34;
  } else {
   HEAP8[$i_09_i_i + 8952 | 0] = $j_08_i_i & 255;
   $j_1_i_i = $j_08_i_i;
  }
  $38 = $i_09_i_i + 1 | 0;
  if (($38 | 0) < 641) {
   $j_08_i_i = $j_1_i_i;
   $i_09_i_i = $38;
  } else {
   break;
  }
 }
 if ((label | 0) == 49) {
  ___assert_func(3248 | 0 | 0, 73 | 0, 6448 | 0 | 0, 3360 | 0 | 0);
  return 0 | 0;
 }
 HEAP32[($world + 102468 | 0) >> 2] = 0;
 HEAP32[($world + 102472 | 0) >> 2] = 0;
 HEAP32[($world + 102476 | 0) >> 2] = 0;
 HEAP32[($world + 102864 | 0) >> 2] = 0;
 HEAP32[($world + 102872 | 0) >> 2] = -1;
 $46 = $world + 102884 | 0;
 HEAP32[$46 >> 2] = 16;
 HEAP32[($world + 102880 | 0) >> 2] = 0;
 $48 = _malloc(576) | 0;
 $50 = $world + 102876 | 0;
 HEAP32[$50 >> 2] = $48;
 _memset($48 | 0 | 0, 0 | 0 | 0, (HEAP32[$46 >> 2] | 0) * 36 & -1 | 0 | 0);
 $54 = (HEAP32[$46 >> 2] | 0) - 1 | 0;
 if (($54 | 0) > 0) {
  $i_05_i_i_i = 0;
  while (1) {
   $56 = $i_05_i_i_i + 1 | 0;
   HEAP32[((HEAP32[$50 >> 2] | 0) + ($i_05_i_i_i * 36 & -1) + 20 | 0) >> 2] = $56;
   HEAP32[((HEAP32[$50 >> 2] | 0) + ($i_05_i_i_i * 36 & -1) + 32 | 0) >> 2] = -1;
   $62 = (HEAP32[$46 >> 2] | 0) - 1 | 0;
   if (($56 | 0) < ($62 | 0)) {
    $i_05_i_i_i = $56;
   } else {
    $_lcssa_i_i_i = $62;
    break;
   }
  }
 } else {
  $_lcssa_i_i_i = $54;
 }
 HEAP32[((HEAP32[$50 >> 2] | 0) + ($_lcssa_i_i_i * 36 & -1) + 20 | 0) >> 2] = -1;
 HEAP32[((HEAP32[$50 >> 2] | 0) + (((HEAP32[$46 >> 2] | 0) - 1 | 0) * 36 & -1) + 32 | 0) >> 2] = -1;
 _memset($world + 102888 | 0 | 0 | 0, 0 | 0 | 0, 16 | 0 | 0);
 HEAP32[($world + 102920 | 0) >> 2] = 16;
 HEAP32[($world + 102924 | 0) >> 2] = 0;
 HEAP32[($world + 102916 | 0) >> 2] = _malloc(192) | 0;
 HEAP32[($world + 102908 | 0) >> 2] = 16;
 HEAP32[($world + 102912 | 0) >> 2] = 0;
 HEAP32[($world + 102904 | 0) >> 2] = _malloc(64) | 0;
 HEAP32[($world + 102932 | 0) >> 2] = 0;
 HEAP32[($world + 102936 | 0) >> 2] = 0;
 HEAP32[($world + 102940 | 0) >> 2] = 104;
 HEAP32[($world + 102944 | 0) >> 2] = 96;
 $87 = $world + 102948 | 0;
 HEAP32[($world + 102980 | 0) >> 2] = 0;
 HEAP32[($world + 102984 | 0) >> 2] = 0;
 _memset($87 | 0 | 0, 0 | 0 | 0, 20 | 0 | 0);
 HEAP8[$world + 102992 | 0] = 1;
 HEAP8[$world + 102993 | 0] = 1;
 HEAP8[$world + 102994 | 0] = 0;
 HEAP8[$world + 102995 | 0] = 1;
 $96 = $world + 102976 | 0;
 HEAP8[$96] = 1;
 $97 = $world + 102968 | 0;
 HEAP32[($97 | 0) >> 2] = 0;
 HEAP32[($97 + 4 | 0) >> 2] = -1054867456;
 $98 = $world + 102868 | 0;
 HEAP32[$98 >> 2] = 4;
 HEAPF32[($world + 102988 | 0) >> 2] = +0;
 HEAP32[$87 >> 2] = $14;
 _memset($world + 102996 | 0 | 0 | 0, 0 | 0 | 0, 32 | 0 | 0);
 HEAP8[$96] = 0;
 HEAP32[($bd + 44 | 0) >> 2] = 0;
 _memset($bd + 4 | 0 | 0 | 0, 0 | 0 | 0, 32 | 0 | 0);
 HEAP8[$bd + 36 | 0] = 1;
 HEAP8[$bd + 37 | 0] = 1;
 HEAP8[$bd + 38 | 0] = 0;
 HEAP8[$bd + 39 | 0] = 0;
 HEAP32[($bd | 0) >> 2] = 0;
 HEAP8[$bd + 40 | 0] = 1;
 HEAPF32[($bd + 48 | 0) >> 2] = +1;
 $112 = __ZN16b2BlockAllocator8AllocateEi($14, 152) | 0;
 if (($112 | 0) == 0) {
  $116 = 0;
 } else {
  $115 = $112;
  __ZN6b2BodyC2EPK9b2BodyDefP7b2World($115, $bd, $world);
  $116 = $115;
 }
 HEAP32[($116 + 92 | 0) >> 2] = 0;
 $118 = $world + 102952 | 0;
 HEAP32[($116 + 96 | 0) >> 2] = HEAP32[$118 >> 2] | 0;
 $121 = HEAP32[$118 >> 2] | 0;
 if (!(($121 | 0) == 0)) {
  HEAP32[($121 + 92 | 0) >> 2] = $116;
 }
 HEAP32[$118 >> 2] = $116;
 $126 = $world + 102960 | 0;
 HEAP32[$126 >> 2] = (HEAP32[$126 >> 2] | 0) + 1 | 0;
 HEAP32[($shape | 0) >> 2] = 8016 | 0;
 HEAP32[($shape + 4 | 0) >> 2] = 1;
 HEAPF32[($shape + 8 | 0) >> 2] = +.009999999776482582;
 _memset($shape + 28 | 0 | 0 | 0, 0 | 0 | 0, 18 | 0 | 0);
 $135 = $shape + 12 | 0;
 HEAP32[($135 | 0) >> 2] = -1038090240;
 HEAP32[($135 + 4 | 0) >> 2] = 0;
 $137 = $shape + 20 | 0;
 HEAP32[($137 | 0) >> 2] = 1109393408;
 HEAP32[($137 + 4 | 0) >> 2] = 0;
 HEAP8[$shape + 44 | 0] = 0;
 HEAP8[$shape + 45 | 0] = 0;
 HEAP16[($def_i + 22 | 0) >> 1] = 1;
 HEAP16[($def_i + 24 | 0) >> 1] = -1;
 HEAP16[($def_i + 26 | 0) >> 1] = 0;
 HEAP32[($def_i + 4 | 0) >> 2] = 0;
 HEAPF32[($def_i + 8 | 0) >> 2] = +.20000000298023224;
 HEAPF32[($def_i + 12 | 0) >> 2] = +0;
 HEAP8[$def_i + 20 | 0] = 0;
 HEAP32[($def_i | 0) >> 2] = $shape | 0;
 HEAPF32[($def_i + 16 | 0) >> 2] = +0;
 __ZN6b2Body13CreateFixtureEPK12b2FixtureDef($116, $def_i);
 HEAP32[($shape1 | 0) >> 2] = 7968 | 0;
 HEAP32[($shape1 + 4 | 0) >> 2] = 2;
 HEAPF32[($shape1 + 8 | 0) >> 2] = +.009999999776482582;
 HEAP32[($shape1 + 148 | 0) >> 2] = 4;
 HEAPF32[($shape1 + 20 | 0) >> 2] = +-.5;
 HEAPF32[($shape1 + 24 | 0) >> 2] = +-.5;
 HEAPF32[($shape1 + 28 | 0) >> 2] = +.5;
 HEAPF32[($shape1 + 32 | 0) >> 2] = +-.5;
 HEAPF32[($shape1 + 36 | 0) >> 2] = +.5;
 HEAPF32[($shape1 + 40 | 0) >> 2] = +.5;
 HEAPF32[($shape1 + 44 | 0) >> 2] = +-.5;
 HEAPF32[($shape1 + 48 | 0) >> 2] = +.5;
 HEAPF32[($shape1 + 84 | 0) >> 2] = +0;
 HEAPF32[($shape1 + 88 | 0) >> 2] = +-1;
 HEAPF32[($shape1 + 92 | 0) >> 2] = +1;
 HEAPF32[($shape1 + 96 | 0) >> 2] = +0;
 HEAPF32[($shape1 + 100 | 0) >> 2] = +0;
 HEAPF32[($shape1 + 104 | 0) >> 2] = +1;
 HEAPF32[($shape1 + 108 | 0) >> 2] = +-1;
 HEAPF32[($shape1 + 112 | 0) >> 2] = +0;
 HEAPF32[($shape1 + 12 | 0) >> 2] = +0;
 HEAPF32[($shape1 + 16 | 0) >> 2] = +0;
 $174 = $bd2 + 44 | 0;
 $176 = $bd2 + 36 | 0;
 $177 = $bd2 + 4 | 0;
 $178 = $bd2 + 37 | 0;
 $179 = $bd2 + 38 | 0;
 $180 = $bd2 + 39 | 0;
 $181 = $bd2 | 0;
 $182 = $bd2 + 40 | 0;
 $183 = $bd2 + 48 | 0;
 $185 = $bd2 + 4 | 0;
 $186 = $shape1 | 0;
 $188 = $def_i21 + 22 | 0;
 $189 = $def_i21 + 24 | 0;
 $190 = $def_i21 + 26 | 0;
 $191 = $def_i21 | 0;
 $192 = $def_i21 + 4 | 0;
 $193 = $def_i21 + 8 | 0;
 $194 = $def_i21 + 12 | 0;
 $195 = $def_i21 + 16 | 0;
 $196 = $def_i21 + 20 | 0;
 $x_sroa_1_4_load313755 = +.75;
 $x_sroa_0_0_load303656 = +-7;
 $i_057 = 0;
 L82 : while (1) {
  $y_sroa_1_4_load293550 = $x_sroa_1_4_load313755;
  $y_sroa_0_0_load283451 = $x_sroa_0_0_load303656;
  $j_052 = $i_057;
  while (1) {
   HEAP32[$174 >> 2] = 0;
   _memset($177 | 0 | 0, 0 | 0 | 0, 32 | 0 | 0);
   HEAP8[$176] = 1;
   HEAP8[$178] = 1;
   HEAP8[$179] = 0;
   HEAP8[$180] = 0;
   HEAP8[$182] = 1;
   HEAPF32[$183 >> 2] = +1;
   HEAP32[$181 >> 2] = 2;
   $y_sroa_0_0_insert_insert$1 = (HEAPF32[tempDoublePtr >> 2] = $y_sroa_1_4_load293550, HEAP32[tempDoublePtr >> 2] | 0) | 0;
   HEAP32[($185 | 0) >> 2] = 0 | (HEAPF32[tempDoublePtr >> 2] = $y_sroa_0_0_load283451, HEAP32[tempDoublePtr >> 2] | 0);
   HEAP32[($185 + 4 | 0) >> 2] = $y_sroa_0_0_insert_insert$1;
   if (!(((HEAP32[$98 >> 2] | 0) & 2 | 0) == 0)) {
    label = 65;
    break L82;
   }
   $205 = __ZN16b2BlockAllocator8AllocateEi($14, 152) | 0;
   if (($205 | 0) == 0) {
    $209 = 0;
   } else {
    $208 = $205;
    __ZN6b2BodyC2EPK9b2BodyDefP7b2World($208, $bd2, $world);
    $209 = $208;
   }
   HEAP32[($209 + 92 | 0) >> 2] = 0;
   HEAP32[($209 + 96 | 0) >> 2] = HEAP32[$118 >> 2] | 0;
   $213 = HEAP32[$118 >> 2] | 0;
   if (!(($213 | 0) == 0)) {
    HEAP32[($213 + 92 | 0) >> 2] = $209;
   }
   HEAP32[$118 >> 2] = $209;
   HEAP32[$126 >> 2] = (HEAP32[$126 >> 2] | 0) + 1 | 0;
   HEAP16[$188 >> 1] = 1;
   HEAP16[$189 >> 1] = -1;
   HEAP16[$190 >> 1] = 0;
   HEAP32[$192 >> 2] = 0;
   HEAPF32[$193 >> 2] = +.20000000298023224;
   HEAPF32[$194 >> 2] = +0;
   HEAP8[$196] = 0;
   HEAP32[$191 >> 2] = $186;
   HEAPF32[$195 >> 2] = +5;
   __ZN6b2Body13CreateFixtureEPK12b2FixtureDef($209, $def_i21);
   $223 = $j_052 + 1 | 0;
   if (($223 | 0) < 40) {
    $y_sroa_1_4_load293550 = $y_sroa_1_4_load293550 + +0;
    $y_sroa_0_0_load283451 = $y_sroa_0_0_load283451 + 1.125;
    $j_052 = $223;
   } else {
    break;
   }
  }
  $236 = $i_057 + 1 | 0;
  if (($236 | 0) < 40) {
   $x_sroa_1_4_load313755 = $x_sroa_1_4_load313755 + +1;
   $x_sroa_0_0_load303656 = $x_sroa_0_0_load303656 + +.5625;
   $i_057 = $236;
  } else {
   $i3_042 = 0;
   break;
  }
 }
 if ((label | 0) == 65) {
  ___assert_func(112 | 0 | 0, 109 | 0, 5328 | 0 | 0, 2520 | 0 | 0);
  return 0 | 0;
 }
 while (1) {
  __ZN7b2World4StepEfii($world);
  $i3_042 = $i3_042 + 1 | 0;
  if (($i3_042 | 0) >= ($WARMUP_0 | 0)) {
   break;
  }
 }
 $241 = HEAP32[9656 >> 2] | 0;
 $242 = _llvm_stacksave() | 0;
 $243 = STACKTOP;
 STACKTOP = STACKTOP + ($241 * 4 & -1) | 0;
 STACKTOP = STACKTOP + 7 >> 3 << 3;
 if (($241 | 0) > 0) {
  $i4_038 = 0;
  while (1) {
   $245 = _clock() | 0;
   __ZN7b2World4StepEfii($world);
   HEAP32[($243 + ($i4_038 << 2) | 0) >> 2] = (_clock() | 0) - $245 | 0;
   $i4_038 = $i4_038 + 1 | 0;
   if (($i4_038 | 0) >= (HEAP32[9656 >> 2] | 0 | 0)) {
    break;
   }
  }
 }
 __Z7measurePm($result, $243);
 $260 = +HEAPF32[($result + 4 | 0) >> 2];
 _printf(3480 | 0 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[tempInt >> 3] = +HEAPF32[($result | 0) >> 2], HEAPF64[tempInt + 8 >> 3] = $260, tempInt) | 0) | 0;
 _llvm_stackrestore($242 | 0);
 __ZN7b2WorldD2Ev($world);
 $_0 = 0;
 STACKTOP = __stackBase__;
 return $_0 | 0;
}
function badf() {
 var $9 = Math_fround(0);
 $9 = (HEAP32[tempDoublePtr>>2]=$8,Math_fround(HEAPF32[tempDoublePtr>>2]));
 HEAPF32[$gep23_asptr>>2] = $9;
}
function badf2() {
 var $9 = 0;
 $9 = (HEAPF32[tempDoublePtr>>2]=$8,HEAP32[tempDoublePtr>>2]|0);
 HEAP32[$gep23_asptr>>2] = $9;
}
function fcomp() {
  // de-morgan's laws are not valid on floats, due to NaNs >:(
 if (!($y < $x)) return 5;
 if (!(5 < $x)) return 5;
 if (!($y < 5)) return 5;
 if (!(($a|0) < ($b|0))) return 5;
 if (!(($a|0) < 5)) return 5;
 if (!(5 < ($b|0))) return 5;
 if (!(5 < 5)) return 5;
}
function conditionalizeMe() {
 if ((x > 1) & (x+y+z+w > 12)) {
  b();
 }
 if ((a() > 1) & (x+y+z+w > 12)) {
  b();
 }
 if ((x > 1) & (x+y+z+k() > 12)) {
  b();
 }
 if ((a() > 1) & (x+y+z+k() > 12)) {
  b();
 }
 if ((x > 1) | (x+y+z+w > 12)) {
  b();
 }
 if ((a() > 1) | (x+y+z+w > 12)) {
  b();
 }
 if ((x > 1) | (x+y+z+k() > 12)) {
  b();
 }
 if ((a() > 1) | (x+y+z+k() > 12)) {
  b();
 }
 if ((x+y+z+w > 12) | (x > 1)) {
  b();
 }
 if ((x+y+z+w > 12) | (a() > 1)) {
  b();
 }
 if ((x+y+z+k() > 12) | (x > 1)) {
  b();
 }
 if ((x+y+z+k() > 12) | (a() > 1)) {
  b();
 }
 while ((x > 1) & (x+y+z+w > 12)) {
  b();
 }
 while ((a() > 1) & (x+y+z+w > 12)) {
  b();
 }
 while ((x > 1) & (x+y+z+k() > 12)) {
  b();
 }
 while ((a() > 1) & (x+y+z+k() > 12)) {
  b();
 }
 if (!($sub$i480 >= Math_fround(+0)) | !($sub4$i483 >= Math_fround(+0))) {
  b();
 }
 if (!($sub$i480 >= Math_fround(+0)) | !($sub4$i483 >= Math_fround(HEAPF32[x+y|0]))) {
  b();
 }
 if (x > 10 | (HEAP[20] + 2) > 5) {
  b();
 }
 print( (((HEAP8[a] + HEAP8[b] + HEAP8[c] + HEAP8[d] + HEAP8[e] + HEAP8[f] | 0) > (a % b % c % d)) ? 1 : $el) | ($cheap > 0) ); // conditional does not always emit boolean
 print( (((HEAP8[a] + HEAP8[b] + HEAP8[c] + HEAP8[d] + HEAP8[e] + HEAP8[f] | 0) > (a % b % c % d)) ? 1 : -1) | ($cheap > 0) );
 print( (((HEAP8[a] + HEAP8[b] + HEAP8[c] + HEAP8[d] + HEAP8[e] + HEAP8[f] | 0) > (a % b % c % d)) ? 1 : 0) | ($cheap > 0) ); // this one is safe!
 print( (((HEAP8[a] + HEAP8[b] + HEAP8[c] + HEAP8[d] + HEAP8[e] + HEAP8[f] | 0) > (a % b % c % d)) ? -1 : 1) | ($cheap > 0) );
 return ((((Math_imul(i6+1, i7) | 0) + 17 | 0) % 5 | 0 | 0) == 0 | ((((Math_imul(i7+1, i7) | 0) + 11 | 0) >>> 0) % 3 | 0 | 0) == 0 | 0) == 0;
}
function bignum() {
 HEAP32[20] = 2779096485 | 0;
 if (!(($2814 | 0) >= 0)) return;
}
function empty() {
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b", "rett", "ret2t", "retf", "i32_8", "tempDoublePtr", "boxx", "_main", "badf", "badf2", "fcomp", "conditionalizeMe", "bignum", "empty"]
