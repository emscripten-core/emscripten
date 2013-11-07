function asm(x, y) {
 x = +x;
 y = y|0;
 var a = 0, b = +0, c = 0;
 var label = 0;
 a = cheez((y+~~x)|0)|0;
 b = a*a;
 fleefl(b|0, a|0);
}
function __Z11printResultPiS_j($needle, $haystack, $len) {
 $needle = $needle | 0;
 $haystack = $haystack | 0;
 $len = $len | 0;
 var $1 = 0, $2 = 0, $3 = 0, $4 = 0, $puts = 0, $7 = 0, $8 = 0, $9 = 0;
 var label = 0;
 var __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 $1 = $needle;
 $2 = $haystack;
 $3 = _bsearch($1, $2, $len, 4, 2);
 $4 = ($3 | 0) == 0;
 if ($4) {
  $puts = _puts(_str | 0);
  STACKTOP = __stackBase__;
  return;
 } else {
  $7 = $3;
  $8 = HEAP32[($7 & 16777215) >> 2] | 0;
  $9 = _printf(__str1 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[(tempInt & 16777215) >> 2] = $8, tempInt));
  STACKTOP = __stackBase__;
  return;
 }
}
function _segment_holding($addr) {
 $addr = $addr | 0;
 var $sp_0 = 0, $2 = 0, $3 = 0, $4 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0;
 var $11 = 0, $12 = 0, $13 = 0, $_0 = 0;
 var label = 0;
 $sp_0 = __gm_ + 444 | 0;
 while (1) {
  $2 = $sp_0 | 0;
  $3 = HEAP32[($2 & 16777215) >> 2] | 0;
  $4 = $3 >>> 0 > $addr >>> 0;
  if (!$4) {
   $6 = $sp_0 + 4 | 0;
   $7 = HEAP32[($6 & 16777215) >> 2] | 0;
   $8 = $3 + $7 | 0;
   $9 = $8 >>> 0 > $addr >>> 0;
   if ($9) {
    $_0 = $sp_0;
    label = 1658;
    break;
   }
  }
  $11 = $sp_0 + 8 | 0;
  $12 = HEAP32[($11 & 16777215) >> 2] | 0;
  $13 = ($12 | 0) == 0;
  if ($13) {
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
 var $__first_addr_i = 0, $__last_addr_i = 0, $__comp_addr_i = 0, $a13 = 0, $a14 = 0, $a18 = 0, $a19 = 0;
 var label = 0;
 var __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 $__first_addr_i = __stackBase__;
 $__last_addr_i = __stackBase__ + 4;
 $a13 = $__first_addr_i;
 $a14 = $__last_addr_i;
 $a18 = $__first_addr_i;
 $a19 = $__last_addr_i;
 STACKTOP = __stackBase__;
 return 0;
}
function exc($this) {
 $this = $this | 0;
 var $1 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $8 = +0, $9 = 0;
 var label = 0;
 if (!__THREW__) {
  $3 = $this | 0;
  $4 = $1 | 0;
  $5 = HEAP32[($4 & 16777215) >> 2] | 0;
  HEAP32[($3 & 16777215) >> 2] = $5;
  $6 = $5 | 0;
  __ZNSt3__114__shared_count12__add_sharedEv($6);
  return;
 } else {
  $8$0 = ___cxa_find_matching_catch(HEAP32[(_llvm_eh_exception.buf & 16777215) >> 2] | 0, HEAP32[(_llvm_eh_exception.buf + 4 & 16777215) >> 2] | 0, []);
  $8$1 = tempRet0;
  $9 = $8$0;
  ___cxa_call_unexpected($9);
 }
}
function label() {
 var $1 = 0, $2 = 0;
 $1 = f();
 if ($1) {
  g();
 }
 $2 = h();
 L100: if ($2) {
  i();
 }
}
function switchy() {
 var no = 0, yes = 0;
 var a = 0, b = 0;
 while (1) switch (label | 0) {
  case 1:
   no = 100; // eliminatable in theory, but eliminator does not look into switch. must leave def above as well.
   break;
  case 2:
   yes = 111;
   yes = yes*2;
   print(yes);
   yes--;
   print(yes/2);
   continue;
  case 3:
   a = 5;
   b = a;
   break;
 }
}
function tempDouble(a) {
 a = +a;
 var x = +0, y = +0;
 // CastAway can leave things like this as variables no longer needed. We need to identify that x's value has no side effects so it can be completely cleaned up
 x = (HEAP32[((tempDoublePtr)>>2)]=((HEAP32[(($_sroa_0_0__idx1)>>2)])|0),HEAP32[(((tempDoublePtr)+(4))>>2)]=((HEAP32[((($_sroa_0_0__idx1)+(4))>>2)])|0),(+(HEAPF64[(tempDoublePtr)>>3])));
 y = a*a;
 f(y);
}
function __ZN23b2EdgeAndPolygonContact8EvaluateEP10b2ManifoldRK11b2TransformS4_($this, $manifold, $xfA, $xfB) {
 $this = $this | 0;
 $manifold = $manifold | 0;
 $xfA = $xfA | 0;
 $xfB = $xfB | 0;
 var $collider_i = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0;
 var $8 = 0, $9 = 0, $10 = 0, $11 = 0;
 var label = 0;
 var __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 256 | 0;
 $collider_i = __stackBase__ | 0;
 $1 = $this + 48 | 0;
 $2 = HEAP32[$1 >> 2] | 0;
 $3 = $2 + 12 | 0;
 $4 = HEAP32[$3 >> 2] | 0;
 $5 = $4;
 $6 = $this + 52 | 0;
 $7 = HEAP32[$6 >> 2] | 0;
 $8 = $7 + 12 | 0;
 $9 = HEAP32[$8 >> 2] | 0;
 $10 = $9;
 $11 = $collider_i;
 __ZN12b2EPCollider7CollideEP10b2ManifoldPK11b2EdgeShapeRK11b2TransformPK14b2PolygonShapeS7_($collider_i, $manifold, $5, $xfA, $10, $xfB);
 STACKTOP = __stackBase__;
 return;
}
function looop2() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = (i + 1)|0;
  if (condition(helper)) {
   break;
  } else {
   i = helper;
  }
 }
}
function looop3() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = (i + 1)|0;
  if (condition(helper)) {
   i = helper;
  } else {
   break;
  }
 }
}
function looop4() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = (i + 1)|0;
  f(i, helper); // i is used, cannot optimize!
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
  helper = (i + 1)|0;
  g(helper);
  if (condition(i)) { // i is used, cannot optimize!
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
  helper = (i + 1)|0;
  if (condition(helper)) {
   i = helper;
  } else {
   break;
  }
 }
 moar(i); // i is still needed, cannot optimize!
}
function looop6() {
 var i = 0, helper = 0;
 while (1) {
  do_it();
  helper = (i + 1)|0;
  if (condition(helper)) {
   i = helper;
  } else {
   break;
  }
 }
 moar(helper); // this is cool
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
 // empty loop
 while (1) {
 }
}
function looop8() {
 var i = 0, j = 0, a = 0;
 while (1) {
  do_it(i, j);
  a = (i + j)|0;
  if (condition(helper)) {
   break;
  } else {
   i = a; // helper used twice!
   j = a;
  }
 }
}
function multiloop($n_0, $35) {
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
  HEAP16[$39 >> 1] = $_off0;
  $46 = $n_0 - 1 | 0;
  if (($46 | 0) == 0) {
   break;
  } else {
   $n_0 = $46;
   $p_0 = $39;
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
  HEAP16[$39 >> 1] = $p_0; // cannot optimize one, so none
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
  if ($cmp38) {} else {
  }
 }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "__Z11printResultPiS_j", "_segment_holding", "__ZN5identC2EiPKcPci", "_vec2Length", "exc", "label", "confuusion", "tempDouble", "_org_apache_harmony_luni_util_NumberConverter_freeFormat__", "__ZN23b2EdgeAndPolygonContact8EvaluateEP10b2ManifoldRK11b2TransformS4_", "_java_nio_charset_Charset_forNameInternal___java_lang_String", "looop2", "looop3", "looop4", "looop5", "looop6", "looop7", "looop8", "multiloop", "multiloop2", "tempDouble2", "watIf"]

