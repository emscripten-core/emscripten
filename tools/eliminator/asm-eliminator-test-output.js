function asm(x, y) {
 x = +x;
 y = y | 0;
 var a = 0;
 a = cheez(y + ~~x | 0) | 0;
 fleefl(a * a | 0, a | 0);
}
function __Z11printResultPiS_j($needle, $haystack, $len) {
 $needle = $needle | 0;
 $haystack = $haystack | 0;
 $len = $len | 0;
 var $3 = 0, __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 $3 = _bsearch($needle, $haystack, $len, 4, 2);
 if (($3 | 0) == 0) {
  _puts(_str | 0);
  STACKTOP = __stackBase__;
  return;
 } else {
  _printf(__str1 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[(tempInt & 16777215) >> 2] = HEAP32[($3 & 16777215) >> 2] | 0, tempInt));
  STACKTOP = __stackBase__;
  return;
 }
}
function _segment_holding($addr) {
 $addr = $addr | 0;
 var $sp_0 = 0, $3 = 0, $12 = 0, $_0 = 0, label = 0;
 $sp_0 = __gm_ + 444 | 0;
 while (1) {
  $3 = HEAP32[(($sp_0 | 0) & 16777215) >> 2] | 0;
  if (!($3 >>> 0 > $addr >>> 0)) {
   if (($3 + (HEAP32[(($sp_0 + 4 | 0) & 16777215) >> 2] | 0) | 0) >>> 0 > $addr >>> 0) {
    $_0 = $sp_0;
    label = 1658;
    break;
   }
  }
  $12 = HEAP32[(($sp_0 + 8 | 0) & 16777215) >> 2] | 0;
  if (($12 | 0) == 0) {
   $_0 = 0;
   label = 1659;
   break;
  } else {
   $sp_0 = $12;
  }
 }
 if (label == 1659) {
  return $_0;
 } else if (label == 1658) {
  return $_0;
 }
}
function __ZN5identC2EiPKcPci($this, $n, $a) {
 $this = $this | 0;
 $n = $n | 0;
 $a = $a | 0;
 HEAP32[($this & 16777215) >> 2] = __ZTV5ident + 8 | 0;
 HEAP32[($this + 4 & 16777215) >> 2] = 5;
 HEAP32[($this + 8 & 16777215) >> 2] = $n;
 HEAP32[($this + 20 & 16777215) >> 2] = 2147483647;
 HEAP32[($this + 24 & 16777215) >> 2] = 0;
 HEAP32[($this + 28 & 16777215) >> 2] = $a;
 HEAP32[($this + 32 & 16777215) >> 2] = 0;
 HEAP32[($this + 40 & 16777215) >> 2] = 1;
 return;
}
function _vec2Length($this) {
 $this = $this | 0;
 var __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 STACKTOP = __stackBase__;
 return 0;
}
function exc($this) {
 $this = $this | 0;
 var $1 = 0, $5 = 0;
 if (!__THREW__) {
  $5 = HEAP32[(($1 | 0) & 16777215) >> 2] | 0;
  HEAP32[(($this | 0) & 16777215) >> 2] = $5;
  __ZNSt3__114__shared_count12__add_sharedEv($5 | 0);
  return;
 } else {
  $8$0 = ___cxa_find_matching_catch(HEAP32[(_llvm_eh_exception.buf & 16777215) >> 2] | 0, HEAP32[(_llvm_eh_exception.buf + 4 & 16777215) >> 2] | 0, []);
  $8$1 = tempRet0;
  ___cxa_call_unexpected($8$0);
 }
}
function label() {
 if (f()) {
  g();
 }
 L100 : if (h()) {
  i();
 }
}
function switchy() {
 var yes = 0;
 while (1) switch (label | 0) {
 case 1:
  break;
 case 2:
  yes = 111;
  yes = yes * 2;
  print(yes);
  yes--;
  print(yes / 2);
  continue;
 case 3:
  break;
 }
}
function tempDouble(a) {
 a = +a;
 f(a * a);
}
function __ZN23b2EdgeAndPolygonContact8EvaluateEP10b2ManifoldRK11b2TransformS4_($this, $manifold, $xfA, $xfB) {
 $this = $this | 0;
 $manifold = $manifold | 0;
 $xfA = $xfA | 0;
 $xfB = $xfB | 0;
 var __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 256 | 0;
 __ZN12b2EPCollider7CollideEP10b2ManifoldPK11b2EdgeShapeRK11b2TransformPK14b2PolygonShapeS7_(__stackBase__ | 0, $manifold, HEAP32[((HEAP32[($this + 48 | 0) >> 2] | 0) + 12 | 0) >> 2] | 0, $xfA, HEAP32[((HEAP32[($this + 52 | 0) >> 2] | 0) + 12 | 0) >> 2] | 0, $xfB);
 STACKTOP = __stackBase__;
 return;
}
function looop2() {
 var i = 0;
 while (1) {
  do_it();
  i = i + 1 | 0;
  if (condition(i)) {
   break;
  }
 }
}
function looop3() {
 var i = 0;
 while (1) {
  do_it();
  i = i + 1 | 0;
  if (!condition(i)) {
   break;
  }
 }
}
function looop4() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = i + 1 | 0;
  f(i, helper);
  if (condition()) {
   i = helper;
  } else {
   break;
  }
 }
}
function looop4b() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = i + 1 | 0;
  g(helper);
  if (condition(i)) {
   i = helper;
  } else {
   break;
  }
 }
}
function looop5() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = i + 1 | 0;
  if (condition(helper)) {
   i = helper;
  } else {
   break;
  }
 }
 moar(i);
}
function looop6() {
 var i = 0;
 while (1) {
  do_it();
  i = i + 1 | 0;
  if (!condition(i)) {
   break;
  }
 }
 moar(i);
}
function looop7() {
 var $old_0_i107_i = 0, $current_0_i108_i = 0, $696 = 0;
 $old_0_i107_i = $draw_left_i;
 while (1) {
  $current_0_i108_i = HEAP32[$old_0_i107_i >> 2] | 0;
  if (($current_0_i108_i | 0) == 0) {
   break;
  }
  $696 = $current_0_i108_i + 4 | 0;
  if (($current_0_i108_i | 0) == ($P_3207_i | 0)) {
   break;
  } else {
   $old_0_i107_i = $696;
  }
 }
 HEAP32[$old_0_i107_i >> 2] = HEAP32[$696 >> 2] | 0;
 while (1) {}
}
function looop8() {
 var i = 0, j = 0, a = 0;
 while (1) {
  do_it(i, j);
  a = i + j | 0;
  if (condition(helper)) {
   break;
  } else {
   i = a;
   j = a;
  }
 }
}
function multiloop($n_0, $35) {
 $n_0 = $n_0 | 0;
 $35 = $35 | 0;
 var $p_0 = 0, $41 = 0;
 $n_0 = $35;
 $p_0 = (HEAP32[$15 >> 2] | 0) + ($35 << 1) | 0;
 while (1) {
  $p_0 = $p_0 - 2 | 0;
  $41 = HEAPU16[$p_0 >> 1] | 0;
  if ($41 >>> 0 < $2 >>> 0) {
   $_off0 = 0;
  } else {
   $_off0 = $41 - $2 & 65535;
  }
  HEAP16[$p_0 >> 1] = $_off0;
  $n_0 = $n_0 - 1 | 0;
  if (($n_0 | 0) == 0) {
   break;
  }
 }
}
function multiloop2($n_0, $35) {
 $n_0 = $n_0 | 0;
 $35 = $35 | 0;
 var $p_0 = 0, $39 = 0, $41 = 0, $46 = 0;
 $n_0 = $35;
 $p_0 = (HEAP32[$15 >> 2] | 0) + ($35 << 1) | 0;
 while (1) {
  $39 = $p_0 - 2 | 0;
  $41 = HEAPU16[$39 >> 1] | 0;
  if ($41 >>> 0 < $2 >>> 0) {
   $_off0 = 0;
  } else {
   $_off0 = $41 - $2 & 65535;
  }
  HEAP16[$39 >> 1] = $p_0;
  $46 = $n_0 - 1 | 0;
  if (($46 | 0) == 0) {
   break;
  } else {
   $n_0 = $46;
   $p_0 = $39;
  }
 }
}
function tempDouble2($46, $14, $28, $42, $20, $32, $45) {
 $46 = $46 | 0;
 $14 = $14 | 0;
 $28 = $28 | 0;
 $42 = $42 | 0;
 $20 = $20 | 0;
 $32 = $32 | 0;
 $45 = $45 | 0;
 var $46 = 0, $_sroa_06_0_insert_insert$1 = 0;
 $46 = (HEAPF32[tempDoublePtr >> 2] = ($14 < $28 ? $14 : $28) - $42, HEAP32[tempDoublePtr >> 2] | 0);
 $_sroa_06_0_insert_insert$1 = (HEAPF32[tempDoublePtr >> 2] = ($20 < $32 ? $20 : $32) - $42, HEAP32[tempDoublePtr >> 2] | 0) | 0;
 HEAP32[$45 >> 2] = 0 | $46;
 HEAP32[$45 + 4 >> 2] = $_sroa_06_0_insert_insert$1;
 HEAP32[$45 + 8 >> 2] = $_sroa_06_0_insert_insert$1;
}
function watIf() {
 while (1) {
  if ($cmp38) {} else {}
 }
}
function select2($foundBase_0_off0) {
 $foundBase_0_off0 = $foundBase_0_off0 | 0;
 var $call24 = 0;
 $call24 = MUST_RUN() | 0;
 STACKTOP = sp;
 return ($foundBase_0_off0 ? 0 : $call24) | 0;
}
function binary(x) {
 x = x | 0;
 memset(f(x)) | 0;
 +dmemset(f(x));
}

