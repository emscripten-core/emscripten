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
  $8$0 = ___cxa_find_matching_catch(HEAP32[(_llvm_eh_exception_buf & 16777215) >> 2] | 0, HEAP32[(_llvm_eh_exception_buf + 4 & 16777215) >> 2] | 0);
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
 var yes1 = 0, yes = 0;
 var a = 0, b = 0;
 while (1) switch (label | 0) {
  case 1:
   yes1 = 100;
   break;
  case 2:
   yes = 111;
   yes = yes*2;
   print(yes);
   yes = yes + 1;
   print(yes/2);
   continue;
  case 3:
   a = 5;
   b = a;
   break;
 }
}
function switchy2($inFormat) {
 $inFormat = $inFormat | 0;
 var label = 0;
 switch ($inFormat | 0) {
 case 14:
  {
   return 5;
   break;
  }
 case 10:
 case 11:
 case 12:
 case 22:
  {
   label = 4;
   break;
  }
 default:
  {}
 }
 if ((label | 0) == 4) {
  return 8;
 }
 return 1;
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
 var $_sroa_06_0_insert_insert$1 = 0;
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
function select2($foundBase_0_off0) {
 $foundBase_0_off0 = $foundBase_0_off0 | 0;
 var $call24 = 0, $retval_0 = 0;
 $call24 = MUST_RUN() | 0;
 $retval_0 = $foundBase_0_off0 ? 0 : $call24;
 STACKTOP = sp;
 return $retval_0 | 0;
}
function binary(x) {
  x = x | 0;
  var y = 0, z = 0;
  y = f(x);
  memset(y) | 0;
  z = f(x);
  +dmemset(z);
}
function cute($this, $outImage) {
 $this = $this | 0;
 $outImage = $outImage | 0;
 var $retval = 0, $this_addr = 0, $outImage_addr = 0, $width = 0, $height = 0, $bit_depth = 0, $color_type = 0, $data = 0, $bpl = 0, $y = 0, $i = 0, $color_table_size = 0, $y76 = 0, $p = 0, $end = 0, $this1 = 0, $state = 0, $0 = 0, $cmp = 0, $state2 = 0;
 var $1 = 0, $cmp3 = 0, $call = 0, $state5 = 0, $row_pointers = 0, $png_ptr = 0, $2 = 0, $call7 = 0, $arraydecay = 0, $call8 = 0, $tobool = 0, $png_ptr10 = 0, $info_ptr = 0, $end_info = 0, $row_pointers11 = 0, $3 = 0, $isnull = 0, $4 = 0, $png_ptr12 = 0, $state13 = 0;
 var $5 = 0, $png_ptr15 = 0, $6 = 0, $info_ptr16 = 0, $7 = 0, $gamma = 0, $8 = +0, $9 = 0, $call17 = 0, $png_ptr19 = 0, $info_ptr20 = 0, $end_info21 = 0, $row_pointers22 = 0, $10 = 0, $isnull23 = 0, $11 = 0, $png_ptr26 = 0, $state27 = 0, $png_ptr29 = 0, $12 = 0;
 var $info_ptr30 = 0, $13 = 0, $call31 = 0, $14 = 0, $call32 = 0, $15 = 0, $call33 = 0, $16 = 0, $17$0 = 0, $17$1 = 0, $18 = 0, $19 = 0, $20 = 0, $call34 = 0, $21 = 0, $row_pointers35 = 0, $22 = 0, $23 = 0, $cmp36 = 0, $24 = 0;
 var $25 = 0, $26 = 0, $mul = 0, $add_ptr = 0, $27 = 0, $row_pointers37 = 0, $28 = 0, $arrayidx = 0, $29 = 0, $inc = 0, $png_ptr38 = 0, $30 = 0, $row_pointers39 = 0, $31 = 0, $32 = 0, $png_ptr40 = 0, $33 = 0, $info_ptr41 = 0, $34 = 0, $call42 = 0;
 var $35 = 0, $png_ptr43 = 0, $36 = 0, $info_ptr44 = 0, $37 = 0, $call45 = 0, $state46 = 0, $png_ptr47 = 0, $38 = 0, $end_info48 = 0, $39 = 0, $end_info49 = 0, $40 = 0, $41 = 0, $readTexts = 0, $42 = 0, $call51 = 0, $sub = 0, $cmp52 = 0, $43 = 0;
 var $readTexts54 = 0, $44 = 0, $45 = 0, $call55 = 0, $readTexts56 = 0, $46 = 0, $47 = 0, $add = 0, $call57 = 0, $48 = 0, $add59 = 0, $png_ptr61 = 0, $info_ptr62 = 0, $end_info63 = 0, $row_pointers64 = 0, $49 = 0, $isnull65 = 0, $50 = 0, $png_ptr68 = 0, $state69 = 0;
 var $51 = 0, $cmp70 = 0, $52 = 0, $call72 = 0, $cmp73 = 0, $53 = 0, $call75 = 0, $54 = 0, $55 = 0, $cmp78 = 0, $56 = 0, $57 = 0, $58 = 0, $mul80 = 0, $add_ptr81 = 0, $59 = 0, $60 = 0, $add_ptr82 = 0, $61 = 0, $62 = 0;
 var $cmp83 = 0, $63 = 0, $64 = 0, $conv = 0, $65 = 0, $cmp84 = 0, $66 = 0, $67 = 0, $incdec_ptr = 0, $68 = 0, $inc88 = 0, $69 = 0, label = 0, setjmpLabel = 0, setjmpTable = 0;
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32 | 0;
 label = 1;
 setjmpLabel = 0;
 setjmpTable = STACKTOP;
 STACKTOP = STACKTOP + 168 | 0;
 HEAP32[setjmpTable >> 2] = 0;
 while (1) switch (label | 0) {
 case 1:
  $width = sp | 0;
  $height = sp + 8 | 0;
  $bit_depth = sp + 16 | 0;
  $color_type = sp + 24 | 0;
  $this_addr = $this;
  $outImage_addr = $outImage;
  $this1 = $this_addr;
  $state = $this1 + 32 | 0;
  $0 = HEAP32[$state >> 2] | 0;
  $cmp = ($0 | 0) == 3;
  if ($cmp) {
   label = 2;
   break;
  } else {
   label = 3;
   break;
  }
 case 2:
  $retval = 0;
  label = 37;
  break;
 case 3:
  $state2 = $this1 + 32 | 0;
  $1 = HEAP32[$state2 >> 2] | 0;
  $cmp3 = ($1 | 0) == 0;
  if ($cmp3) {
   label = 4;
   break;
  } else {
   label = 6;
   break;
  }
 case 4:
  $call = invoke_ii(900, $this1 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  if ($call) {
   label = 6;
   break;
  } else {
   label = 5;
   break;
  }
 case 5:
  $state5 = $this1 + 32 | 0;
  HEAP32[$state5 >> 2] = 3;
  $retval = 0;
  label = 37;
  break;
 case 6:
  $row_pointers = $this1 + 28 | 0;
  HEAP32[$row_pointers >> 2] = 0;
  $png_ptr = $this1 + 16 | 0;
  $2 = HEAP32[$png_ptr >> 2] | 0;
  $call7 = invoke_iiii(30, $2 | 0, 2638 | 0, 156 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $arraydecay = $call7 | 0;
  $call8 = _saveSetjmp($arraydecay | 0, label, setjmpTable) | 0;
  label = 38;
  break;
 case 38:
  $tobool = ($call8 | 0) != 0;
  if ($tobool) {
   label = 7;
   break;
  } else {
   label = 10;
   break;
  }
 case 7:
  $png_ptr10 = $this1 + 16 | 0;
  $info_ptr = $this1 + 20 | 0;
  $end_info = $this1 + 24 | 0;
  invoke_viii(640, $png_ptr10 | 0, $info_ptr | 0, $end_info | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $row_pointers11 = $this1 + 28 | 0;
  $3 = HEAP32[$row_pointers11 >> 2] | 0;
  $isnull = ($3 | 0) == 0;
  if ($isnull) {
   label = 9;
   break;
  } else {
   label = 8;
   break;
  }
 case 8:
  $4 = $3;
  invoke_vi(926, $4 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  label = 9;
  break;
 case 9:
  $png_ptr12 = $this1 + 16 | 0;
  HEAP32[$png_ptr12 >> 2] = 0;
  $state13 = $this1 + 32 | 0;
  HEAP32[$state13 >> 2] = 3;
  $retval = 0;
  label = 37;
  break;
 case 10:
  $5 = $outImage_addr;
  $png_ptr15 = $this1 + 16 | 0;
  $6 = HEAP32[$png_ptr15 >> 2] | 0;
  $info_ptr16 = $this1 + 20 | 0;
  $7 = HEAP32[$info_ptr16 >> 2] | 0;
  $gamma = $this1 | 0;
  $8 = +HEAPF32[$gamma >> 2];
  invoke_viiif(2, $5 | 0, $6 | 0, $7 | 0, +$8);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $9 = $outImage_addr;
  $call17 = invoke_ii(832, $9 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  if ($call17) {
   label = 11;
   break;
  } else {
   label = 14;
   break;
  }
 case 11:
  $png_ptr19 = $this1 + 16 | 0;
  $info_ptr20 = $this1 + 20 | 0;
  $end_info21 = $this1 + 24 | 0;
  invoke_viii(640, $png_ptr19 | 0, $info_ptr20 | 0, $end_info21 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $row_pointers22 = $this1 + 28 | 0;
  $10 = HEAP32[$row_pointers22 >> 2] | 0;
  $isnull23 = ($10 | 0) == 0;
  if ($isnull23) {
   label = 13;
   break;
  } else {
   label = 12;
   break;
  }
 case 12:
  $11 = $10;
  invoke_vi(926, $11 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  label = 13;
  break;
 case 13:
  $png_ptr26 = $this1 + 16 | 0;
  HEAP32[$png_ptr26 >> 2] = 0;
  $state27 = $this1 + 32 | 0;
  HEAP32[$state27 >> 2] = 3;
  $retval = 0;
  label = 37;
  break;
 case 14:
  $png_ptr29 = $this1 + 16 | 0;
  $12 = HEAP32[$png_ptr29 >> 2] | 0;
  $info_ptr30 = $this1 + 20 | 0;
  $13 = HEAP32[$info_ptr30 >> 2] | 0;
  $call31 = invoke_iiiiiiiiii(2, $12 | 0, $13 | 0, $width | 0, $height | 0, $bit_depth | 0, $color_type | 0, 0 | 0, 0 | 0, 0 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $14 = $outImage_addr;
  $call32 = invoke_ii(850, $14 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $data = $call32;
  $15 = $outImage_addr;
  $call33 = invoke_ii(284, $15 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $bpl = $call33;
  $16 = HEAP32[$height >> 2] | 0;
  $17$0 = invoke_iii(860, $16 | 0, 4 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $17$1 = tempRet0;
  $18 = $17$1;
  $19 = $17$0;
  $20 = $18 ? -1 : $19;
  $call34 = invoke_ii(550, $20 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $21 = $call34;
  $row_pointers35 = $this1 + 28 | 0;
  HEAP32[$row_pointers35 >> 2] = $21;
  $y = 0;
  label = 15;
  break;
 case 15:
  $22 = $y;
  $23 = HEAP32[$height >> 2] | 0;
  $cmp36 = $22 >>> 0 < $23 >>> 0;
  if ($cmp36) {
   label = 16;
   break;
  } else {
   label = 18;
   break;
  }
 case 16:
  $24 = $data;
  $25 = $y;
  $26 = $bpl;
  $mul = Math_imul($25, $26) | 0;
  $add_ptr = $24 + $mul | 0;
  $27 = $y;
  $row_pointers37 = $this1 + 28 | 0;
  $28 = HEAP32[$row_pointers37 >> 2] | 0;
  $arrayidx = $28 + ($27 << 2) | 0;
  HEAP32[$arrayidx >> 2] = $add_ptr;
  label = 17;
  break;
 case 17:
  $29 = $y;
  $inc = $29 + 1 | 0;
  $y = $inc;
  label = 15;
  break;
 case 18:
  $png_ptr38 = $this1 + 16 | 0;
  $30 = HEAP32[$png_ptr38 >> 2] | 0;
  $row_pointers39 = $this1 + 28 | 0;
  $31 = HEAP32[$row_pointers39 >> 2] | 0;
  invoke_vii(858, $30 | 0, $31 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $32 = $outImage_addr;
  $png_ptr40 = $this1 + 16 | 0;
  $33 = HEAP32[$png_ptr40 >> 2] | 0;
  $info_ptr41 = $this1 + 20 | 0;
  $34 = HEAP32[$info_ptr41 >> 2] | 0;
  $call42 = invoke_iii(690, $33 | 0, $34 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  invoke_vii(1890, $32 | 0, $call42 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $35 = $outImage_addr;
  $png_ptr43 = $this1 + 16 | 0;
  $36 = HEAP32[$png_ptr43 >> 2] | 0;
  $info_ptr44 = $this1 + 20 | 0;
  $37 = HEAP32[$info_ptr44 >> 2] | 0;
  $call45 = invoke_iii(256, $36 | 0, $37 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  invoke_vii(2126, $35 | 0, $call45 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $state46 = $this1 + 32 | 0;
  HEAP32[$state46 >> 2] = 2;
  $png_ptr47 = $this1 + 16 | 0;
  $38 = HEAP32[$png_ptr47 >> 2] | 0;
  $end_info48 = $this1 + 24 | 0;
  $39 = HEAP32[$end_info48 >> 2] | 0;
  invoke_vii(36, $38 | 0, $39 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $end_info49 = $this1 + 24 | 0;
  $40 = HEAP32[$end_info49 >> 2] | 0;
  invoke_vii(2752, $this1 | 0, $40 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $i = 0;
  label = 19;
  break;
 case 19:
  $41 = $i;
  $readTexts = $this1 + 12 | 0;
  $42 = $readTexts;
  $call51 = invoke_ii(618, $42 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $sub = $call51 - 1 | 0;
  $cmp52 = ($41 | 0) < ($sub | 0);
  if ($cmp52) {
   label = 20;
   break;
  } else {
   label = 22;
   break;
  }
 case 20:
  $43 = $outImage_addr;
  $readTexts54 = $this1 + 12 | 0;
  $44 = $readTexts54;
  $45 = $i;
  $call55 = invoke_iii(502, $44 | 0, $45 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $readTexts56 = $this1 + 12 | 0;
  $46 = $readTexts56;
  $47 = $i;
  $add = $47 + 1 | 0;
  $call57 = invoke_iii(502, $46 | 0, $add | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  invoke_viii(550, $43 | 0, $call55 | 0, $call57 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  label = 21;
  break;
 case 21:
  $48 = $i;
  $add59 = $48 + 2 | 0;
  $i = $add59;
  label = 19;
  break;
 case 22:
  $png_ptr61 = $this1 + 16 | 0;
  $info_ptr62 = $this1 + 20 | 0;
  $end_info63 = $this1 + 24 | 0;
  invoke_viii(640, $png_ptr61 | 0, $info_ptr62 | 0, $end_info63 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $row_pointers64 = $this1 + 28 | 0;
  $49 = HEAP32[$row_pointers64 >> 2] | 0;
  $isnull65 = ($49 | 0) == 0;
  if ($isnull65) {
   label = 24;
   break;
  } else {
   label = 23;
   break;
  }
 case 23:
  $50 = $49;
  invoke_vi(926, $50 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  label = 24;
  break;
 case 24:
  $png_ptr68 = $this1 + 16 | 0;
  HEAP32[$png_ptr68 >> 2] = 0;
  $state69 = $this1 + 32 | 0;
  HEAP32[$state69 >> 2] = 0;
  $51 = HEAP32[$color_type >> 2] | 0;
  $cmp70 = ($51 | 0) == 3;
  if ($cmp70) {
   label = 25;
   break;
  } else {
   label = 36;
   break;
  }
 case 25:
  $52 = $outImage_addr;
  $call72 = invoke_ii(926, $52 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $cmp73 = ($call72 | 0) == 3;
  if ($cmp73) {
   label = 26;
   break;
  } else {
   label = 36;
   break;
  }
 case 26:
  $53 = $outImage_addr;
  $call75 = invoke_ii(860, $53 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $color_table_size = $call75;
  $y76 = 0;
  label = 27;
  break;
 case 27:
  $54 = $y76;
  $55 = HEAP32[$height >> 2] | 0;
  $cmp78 = ($54 | 0) < ($55 | 0);
  if ($cmp78) {
   label = 28;
   break;
  } else {
   label = 35;
   break;
  }
 case 28:
  $56 = $data;
  $57 = $y76;
  $58 = $bpl;
  $mul80 = Math_imul($57, $58) | 0;
  $add_ptr81 = $56 + $mul80 | 0;
  $p = $add_ptr81;
  $59 = $p;
  $60 = HEAP32[$width >> 2] | 0;
  $add_ptr82 = $59 + $60 | 0;
  $end = $add_ptr82;
  label = 29;
  break;
 case 29:
  $61 = $p;
  $62 = $end;
  $cmp83 = $61 >>> 0 < $62 >>> 0;
  if ($cmp83) {
   label = 30;
   break;
  } else {
   label = 33;
   break;
  }
 case 30:
  $63 = $p;
  $64 = HEAP8[$63] | 0;
  $conv = $64 & 255;
  $65 = $color_table_size;
  $cmp84 = ($conv | 0) >= ($65 | 0);
  if ($cmp84) {
   label = 31;
   break;
  } else {
   label = 32;
   break;
  }
 case 31:
  $66 = $p;
  HEAP8[$66] = 0;
  label = 32;
  break;
 case 32:
  $67 = $p;
  $incdec_ptr = $67 + 1 | 0;
  $p = $incdec_ptr;
  label = 29;
  break;
 case 33:
  label = 34;
  break;
 case 34:
  $68 = $y76;
  $inc88 = $68 + 1 | 0;
  $y76 = $inc88;
  label = 27;
  break;
 case 35:
  label = 36;
  break;
 case 36:
  $retval = 1;
  label = 37;
  break;
 case 37:
  $69 = $retval;
  STACKTOP = sp;
  return $69 | 0;
 case -1:
  if ((setjmpLabel | 0) == 6) {
   $call8 = threwValue;
   label = 38;
  }
  __THREW__ = threwValue = 0;
  break;
 }
 return 0;
}
function selfAssign() {
 var i1 = 0;
 i1 = HEAP32[2] | 0;
 HEAP32[2] = i1 + 1;
 if (waka) {
  return (STACKTOP = STACKTOP, 0);
 }
 STACKTOP = STACKTOP;
 return i1 & 16384 | 0;
}
function elimOneLoopVar($argc, $argv) {
 $argc = $argc | 0;
 $argv = $argv | 0;
 var $0 = 0, $1 = 0, $arg$0 = 0, $arrayidx = 0, $call10 = Math_fround(0), $cmp = 0, $cmp11 = 0, $cmp119 = 0, $cmp12 = 0, $cmp7 = 0, $conv = 0, $conv8 = Math_fround(0), $conv9 = Math_fround(0), $curri$012 = 0, $inc = 0, $inc14$primes$0 = 0, $inc16 = 0, $j$010 = 0, $j$010$phi = 0, $ok$0 = 0;
 var $primes$011 = 0, $rem = 0, $retval$0 = 0, $sub = 0, $vararg_buffer1 = 0, label = 0, sp = 0;
 $curri$012 = 2;
 $primes$011 = 0;
 while (1) {
  $conv9 = Math_fround($curri$012 | 0);
  $call10 = Math_fround(Math_sqrt(Math_fround($conv9)));
  $cmp119 = $call10 > Math_fround(+2);
  L15 : do {
   if ($cmp119) {
    $j$010 = 2;
    while (1) {
     $rem = ($curri$012 | 0) % ($j$010 | 0) & -1;
     $cmp12 = ($rem | 0) == 0;
     $inc = $j$010 + 1 | 0;
     if ($cmp12) {
      $ok$0 = 0;
      break L15;
     }
     $conv8 = Math_fround($inc | 0);
     $cmp11 = $conv8 < $call10;
     if ($cmp11) {
      $j$010$phi = $inc;
      $j$010 = $j$010$phi;
     } else {
      $ok$0 = 1;
      break;
     }
    }
   } else {
    $ok$0 = 1;
   }
  } while (0);
  $inc14$primes$0 = $ok$0 + $primes$011 | 0;
  $inc16 = $curri$012 + 1 | 0;
  $cmp7 = ($inc14$primes$0 | 0) < ($arg$0 | 0);
  if ($cmp7) {
   $curri$012 = $inc16;
   $primes$011 = $inc14$primes$0;
  } else {
   break;
  }
 }
 HEAP32[$vararg_buffer1 >> 2] = $curri$012;
 return $retval$0 | 0;
}
function elimOneLoopVar2() {
 var $storemerge3$neg9 = 0, $18 = 0, $25 = 0, $26 = 0, $30 = 0, $jp = 0;
 $storemerge3$neg9 = -1;
 while (1) {
  $25 = $jp + ($26 << 2) | 0;
  HEAP32[$25 >> 2] = ($18 + $storemerge3$neg9 | 0) + (HEAP32[$25 >> 2] | 0) | 0;
  $30 = $26 + 1 | 0;
  if (($30 | 0) == 63) {
   break;
  } else {
   $storemerge3$neg9 = $26 ^ -1; // $26 is a loopvar, use here is dangerous
   $26 = $30;
  }
 }
}
function elimOneLoopVar3() {
 var $storemerge3$neg9 = 0, $18 = 0, $25 = 0, $26 = 0, $30 = 0, $jp = 0;
 $storemerge3$neg9 = -1;
 while (1) {
  $25 = $jp + ($26 << 2) | 0;
  HEAP32[$25 >> 2] = ($18 + $storemerge3$neg9 | 0) + (HEAP32[$25 >> 2] | 0) | 0;
  $30 = $26 + 1 | 0;
  if (($30 | 0) == 63) {
   break;
  } else {
   $storemerge3$neg9 = $30 ^ -1; // $26 is a helper, use here is dangerous
   $26 = $30;
  }
 }
}
function elimOneLoopVar4() {
 var $storemerge3$neg9 = 0, $18 = 0, $25 = 0, $26 = 0, $30 = 0, $jp = 0;
 $storemerge3$neg9 = -1;
 while (1) {
  $25 = $jp + ($26 << 2) | 0;
  HEAP32[$25 >> 2] = ($18 + $storemerge3$neg9 | 0) + (HEAP32[$25 >> 2] | 0) | 0;
  $30 = $26 + 1 | 0;
  if (($30 | 0) == 63) {
   break;
  } else {
   $storemerge3$neg9 = $18 ^ -1;
   $26 = $30;
  }
 }
}
function elimOneLoopVarStillUsed() {
 var $0 = 0, $1 = 0, $arg$0 = 0, $arrayidx = 0, $call10 = Math_fround(0), $cmp = 0, $cmp11 = 0, $cmp119 = 0, $cmp12 = 0, $cmp7 = 0, $conv = 0, $conv8 = Math_fround(0), $conv9 = Math_fround(0), $curri$012 = 0, $inc = 0, $inc14$primes$0 = 0, $inc16 = 0, $j$010 = 0, $ok$0 = 0;
 var $primes$011 = 0, $rem = 0, $retval$0 = 0, $sub = 0, $vararg_buffer1 = 0, label = 0, sp = 0;
 while (1) {
  $rem = ($curri$012 | 0) % ($j$010 | 0) & -1;
  $cmp12 = ($rem | 0) == 0;
  $inc = $j$010 + 1 | 0;
  if ($cmp12) {
   $ok$0 = 0;
   break;
  }
  $conv8 = Math_fround($inc | 0);
  $cmp11 = $conv8 < $call10;
  if ($cmp11) {
   $j$010 = $inc;
  } else {
   break;
  }
 }
 return $retval$0 | 0;
}
function elimOneLoopVarStillUsedSE() {
 var $0 = 0, $1 = 0, $arg$0 = 0, $arrayidx = 0, $call10 = Math_fround(0), $cmp = 0, $cmp11 = 0, $cmp119 = 0, $cmp12 = 0, $cmp7 = 0, $conv = 0, $conv8 = Math_fround(0), $conv9 = Math_fround(0), $curri$012 = 0, $inc = 0, $inc14$primes$0 = 0, $inc16 = 0, $j$010 = 0, $ok$0 = 0;
 var $primes$011 = 0, $rem = 0, $retval$0 = 0, $sub = 0, $vararg_buffer1 = 0, label = 0, sp = 0;
 while (1) {
  $rem = ($curri$012 | 0) % ($j$010 | 0) & -1;
  $cmp12 = ($rem | 0) == 0;
  $inc = $j$010 + sideeffect() | 0; // side effect!
  if ($cmp12) {
   $ok$0 = 0;
   break;
  }
  $conv8 = Math_fround($inc | 0);
  $cmp11 = $conv8 < $call10;
  if ($cmp11) {
   $j$010 = $inc;
  } else {
   break;
  }
 }
 return $retval$0 | 0;
}
function elimOneLoopVar5() {
 var $storemerge3$neg9 = 0, $18 = 0, $25 = 0, $26 = 0, $30 = 0, $jp = 0;
 $storemerge3$neg9 = -1;
 while (1) {
  $25 = $jp + ($26 << 2) | 0;
  HEAP32[$25 >> 2] = ($18 + $storemerge3$neg9 | 0) + (HEAP32[$25 >> 2] | 0) | 0;
  $30 = $26 + 1 | 0;
  if (($30 | 0) == 63) {
   f($30); // loop var used here, so cannot be easily optimized
   break;
  } else {
   $storemerge3$neg9 = $18 ^ -1;
   $26 = $30;
  }
 }
}
function loopVarWithContinue() {
  var i = 0, inc = 0;
  i = 0;
  while (1) {
    inc = i + 1;
    if (check()) {
      i = i + 1;
      continue;
    }
    work(inc);
    work(i);
    work(inc);
    if (check()) {
      break;
    } else {
      i = inc;
    }
  }
}
function helperExtraUse() {
 var i = 0, inc = 0;
 i = 0;
 while (1) {
  inc = i + 1;
  work(i);
  work(inc);
  if (check()) {
   break;
  } else {
   i = inc;
  }
 }
 return inc;
}
function mixed_up_loop_helpers() {
 var $cmp = 0, $cmp13 = 0, $cmp15 = 0, $cmp2 = 0, $cmp7 = 0, $div = +0, $div11 = +0, $inc = 0, $iterations$0 = 0, $iterations$0$ph = 0, $or$cond = 0, $p$sroa$0$0$ph = +0, $p$sroa$1$0$ph = +0, $p$sroa$1$0$ph$phi = +0, $rem = 0, $sub = +0, $vararg_buffer = 0, $vararg_ptr1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 $vararg_buffer = sp;
 $iterations$0$ph = 0;
 $p$sroa$0$0$ph = +0;
 $p$sroa$1$0$ph = +1;
 L1 : while (1) {
  $iterations$0 = $iterations$0$ph;
  while (1) {
   $inc = $iterations$0 + 1 | 0;
   $cmp = ($inc | 0) < 10;
   if (!$cmp) {
    label = 8;
    break L1;
   }
   HEAPF64[tempDoublePtr >> 3] = $p$sroa$0$0$ph;
   HEAP32[$vararg_buffer >> 2] = HEAP32[tempDoublePtr >> 2];
   HEAP32[$vararg_buffer + 4 >> 2] = HEAP32[tempDoublePtr + 4 >> 2];
   $vararg_ptr1 = $vararg_buffer + 8 | 0;
   HEAPF64[tempDoublePtr >> 3] = $p$sroa$1$0$ph;
   HEAP32[$vararg_ptr1 >> 2] = HEAP32[tempDoublePtr >> 2];
   HEAP32[$vararg_ptr1 + 4 >> 2] = HEAP32[tempDoublePtr + 4 >> 2];
   _printf(8 | 0, $vararg_buffer | 0) | 0;
   $cmp2 = ($iterations$0 | 0) > 0;
   if ($cmp2) {
    break;
   } else {
    $iterations$0 = $inc;
   }
  }
  $sub = -$p$sroa$1$0$ph;
  $rem = ($inc | 0) % 3 & -1;
  $cmp7 = ($rem | 0) == 0;
  if (!$cmp7) {
   $p$sroa$1$0$ph$phi = $p$sroa$0$0$ph;
   $iterations$0$ph = $inc;
   $p$sroa$0$0$ph = $sub;
   $p$sroa$1$0$ph = $p$sroa$1$0$ph$phi;
   continue;
  }
  $div = $p$sroa$1$0$ph * +-.5;
  $div11 = $p$sroa$0$0$ph * +.5;
  $cmp13 = $div == +0;
  $cmp15 = $div11 == +0;
  $or$cond = $cmp13 & $cmp15;
  if ($or$cond) {
   break;
  } else {
   $iterations$0$ph = $inc;
   $p$sroa$0$0$ph = $div;
   $p$sroa$1$0$ph = $div11;
  }
 }
}
function _postProcess() {
 var $w = 0, $z = 0;
 $w = Math_abs($global);
 $z = $w;
 cheez();
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "__Z11printResultPiS_j", "_segment_holding", "__ZN5identC2EiPKcPci", "_vec2Length", "exc", "label", "confuusion", "tempDouble", "_org_apache_harmony_luni_util_NumberConverter_freeFormat__", "__ZN23b2EdgeAndPolygonContact8EvaluateEP10b2ManifoldRK11b2TransformS4_", "_java_nio_charset_Charset_forNameInternal___java_lang_String", "looop2", "looop3", "looop4", "looop5", "looop6", "looop7", "looop8", "multiloop", "multiloop2", "tempDouble2", "watIf", "select2", "binary", "cute", "selfAssign", "elimOneLoopVar", "elimOneLoopVar2", "elimOneLoopVar3", "elimOneLoopVar4", "elimOneLoopVarStillUsed", "elimOneLoopVarStillUsedSE", "elimOneLoopVar5", "helperExtraUse", "mixed_up_loop_helpers", "_postProcess"]

