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
 var $sp_0 = 0, $3 = 0, $_0 = 0, label = 0;
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
  $sp_0 = HEAP32[(($sp_0 + 8 | 0) & 16777215) >> 2] | 0;
  if (($sp_0 | 0) == 0) {
   $_0 = 0;
   label = 1659;
   break;
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
  $8$0 = ___cxa_find_matching_catch(HEAP32[(_llvm_eh_exception_buf & 16777215) >> 2] | 0, HEAP32[(_llvm_eh_exception_buf + 4 & 16777215) >> 2] | 0);
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
  yes = yes + 1;
  print(yes / 2);
  continue;
 case 3:
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
 var i = 0, i$looptemp = 0;
 while (1) {
  do_it();
  i$looptemp = i;
  i = i + 1 | 0;
  f(i$looptemp, i);
  if (!condition()) {
   break;
  }
 }
}
function looop4b() {
 var i = 0, i$looptemp = 0;
 while (1) {
  do_it();
  i$looptemp = i;
  i = i + 1 | 0;
  g(i);
  if (!condition(i$looptemp)) {
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
 var $p_0 = 0, $41 = 0, $p_0$looptemp = 0;
 $n_0 = $35;
 $p_0 = (HEAP32[$15 >> 2] | 0) + ($35 << 1) | 0;
 while (1) {
  $p_0$looptemp = $p_0;
  $p_0 = $p_0 - 2 | 0;
  $41 = HEAPU16[$p_0 >> 1] | 0;
  if ($41 >>> 0 < $2 >>> 0) {
   $_off0 = 0;
  } else {
   $_off0 = $41 - $2 & 65535;
  }
  HEAP16[$p_0 >> 1] = $p_0$looptemp;
  $n_0 = $n_0 - 1 | 0;
  if (($n_0 | 0) == 0) {
   break;
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
function cute($this, $outImage) {
 $this = $this | 0;
 $outImage = $outImage | 0;
 var $retval = 0, $outImage_addr = 0, $width = 0, $height = 0, $bit_depth = 0, $color_type = 0, $data = 0, $bpl = 0, $y = 0, $i = 0, $y76 = 0, $p = 0, $end = 0, $this1 = 0, $call = 0, $call7 = 0, $call8 = 0, $3 = 0, $call17 = 0, $10 = 0, $call32 = 0, $call33 = 0, $17$0 = 0, $call34 = 0, $add_ptr = 0, $32 = 0, $call42 = 0, $35 = 0, $call45 = 0, $41 = 0, $call51 = 0, $43 = 0, $call55 = 0, $call57 = 0, $49 = 0, $call72 = 0, $call75 = 0, label = 0, setjmpLabel = 0, setjmpTable = 0, sp = 0;
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
  $outImage_addr = $outImage;
  $this1 = $this;
  if ((HEAP32[($this1 + 32 | 0) >> 2] | 0 | 0) == 3) {
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
  if ((HEAP32[($this1 + 32 | 0) >> 2] | 0 | 0) == 0) {
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
  HEAP32[($this1 + 32 | 0) >> 2] = 3;
  $retval = 0;
  label = 37;
  break;
 case 6:
  HEAP32[($this1 + 28 | 0) >> 2] = 0;
  $call7 = invoke_iiii(30, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, 2638 | 0, 156 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $call8 = _saveSetjmp($call7 | 0 | 0, label, setjmpTable) | 0;
  label = 38;
  break;
 case 38:
  if (($call8 | 0) != 0) {
   label = 7;
   break;
  } else {
   label = 10;
   break;
  }
 case 7:
  invoke_viii(640, $this1 + 16 | 0 | 0, $this1 + 20 | 0 | 0, $this1 + 24 | 0 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $3 = HEAP32[($this1 + 28 | 0) >> 2] | 0;
  if (($3 | 0) == 0) {
   label = 9;
   break;
  } else {
   label = 8;
   break;
  }
 case 8:
  invoke_vi(926, $3 | 0);
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
  HEAP32[($this1 + 16 | 0) >> 2] = 0;
  HEAP32[($this1 + 32 | 0) >> 2] = 3;
  $retval = 0;
  label = 37;
  break;
 case 10:
  invoke_viiif(2, $outImage_addr | 0, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, HEAP32[($this1 + 20 | 0) >> 2] | 0 | 0, +(+HEAPF32[($this1 | 0) >> 2]));
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $call17 = invoke_ii(832, $outImage_addr | 0) | 0;
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
  invoke_viii(640, $this1 + 16 | 0 | 0, $this1 + 20 | 0 | 0, $this1 + 24 | 0 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $10 = HEAP32[($this1 + 28 | 0) >> 2] | 0;
  if (($10 | 0) == 0) {
   label = 13;
   break;
  } else {
   label = 12;
   break;
  }
 case 12:
  invoke_vi(926, $10 | 0);
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
  HEAP32[($this1 + 16 | 0) >> 2] = 0;
  HEAP32[($this1 + 32 | 0) >> 2] = 3;
  $retval = 0;
  label = 37;
  break;
 case 14:
  invoke_iiiiiiiiii(2, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, HEAP32[($this1 + 20 | 0) >> 2] | 0 | 0, $width | 0, $height | 0, $bit_depth | 0, $color_type | 0, 0 | 0, 0 | 0, 0 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $call32 = invoke_ii(850, $outImage_addr | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $data = $call32;
  $call33 = invoke_ii(284, $outImage_addr | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $bpl = $call33;
  $17$0 = invoke_iii(860, HEAP32[$height >> 2] | 0 | 0, 4 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $call34 = invoke_ii(550, (tempRet0 ? -1 : $17$0) | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  HEAP32[($this1 + 28 | 0) >> 2] = $call34;
  $y = 0;
  label = 15;
  break;
 case 15:
  if ($y >>> 0 < (HEAP32[$height >> 2] | 0) >>> 0) {
   label = 16;
   break;
  } else {
   label = 18;
   break;
  }
 case 16:
  $add_ptr = $data + (Math_imul($y, $bpl) | 0) | 0;
  HEAP32[((HEAP32[($this1 + 28 | 0) >> 2] | 0) + ($y << 2) | 0) >> 2] = $add_ptr;
  label = 17;
  break;
 case 17:
  $y = $y + 1 | 0;
  label = 15;
  break;
 case 18:
  invoke_vii(858, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, HEAP32[($this1 + 28 | 0) >> 2] | 0 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $32 = $outImage_addr;
  $call42 = invoke_iii(690, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, HEAP32[($this1 + 20 | 0) >> 2] | 0 | 0) | 0;
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
  $call45 = invoke_iii(256, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, HEAP32[($this1 + 20 | 0) >> 2] | 0 | 0) | 0;
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
  HEAP32[($this1 + 32 | 0) >> 2] = 2;
  invoke_vii(36, HEAP32[($this1 + 16 | 0) >> 2] | 0 | 0, HEAP32[($this1 + 24 | 0) >> 2] | 0 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  invoke_vii(2752, $this1 | 0, HEAP32[($this1 + 24 | 0) >> 2] | 0 | 0);
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
  $call51 = invoke_ii(618, $this1 + 12 | 0 | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  if (($41 | 0) < ($call51 - 1 | 0 | 0)) {
   label = 20;
   break;
  } else {
   label = 22;
   break;
  }
 case 20:
  $43 = $outImage_addr;
  $call55 = invoke_iii(502, $this1 + 12 | 0 | 0, $i | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $call57 = invoke_iii(502, $this1 + 12 | 0 | 0, $i + 1 | 0 | 0) | 0;
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
  $i = $i + 2 | 0;
  label = 19;
  break;
 case 22:
  invoke_viii(640, $this1 + 16 | 0 | 0, $this1 + 20 | 0 | 0, $this1 + 24 | 0 | 0);
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $49 = HEAP32[($this1 + 28 | 0) >> 2] | 0;
  if (($49 | 0) == 0) {
   label = 24;
   break;
  } else {
   label = 23;
   break;
  }
 case 23:
  invoke_vi(926, $49 | 0);
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
  HEAP32[($this1 + 16 | 0) >> 2] = 0;
  HEAP32[($this1 + 32 | 0) >> 2] = 0;
  if ((HEAP32[$color_type >> 2] | 0 | 0) == 3) {
   label = 25;
   break;
  } else {
   label = 36;
   break;
  }
 case 25:
  $call72 = invoke_ii(926, $outImage_addr | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  if (($call72 | 0) == 3) {
   label = 26;
   break;
  } else {
   label = 36;
   break;
  }
 case 26:
  $call75 = invoke_ii(860, $outImage_addr | 0) | 0;
  if ((__THREW__ | 0) != 0 & (threwValue | 0) != 0) {
   setjmpLabel = _testSetjmp(HEAP32[__THREW__ >> 2] | 0, setjmpTable) | 0;
   if ((setjmpLabel | 0) > 0) {
    label = -1;
    break;
   } else return 0 | 0;
  }
  __THREW__ = threwValue = 0;
  $y76 = 0;
  label = 27;
  break;
 case 27:
  if (($y76 | 0) < (HEAP32[$height >> 2] | 0 | 0)) {
   label = 28;
   break;
  } else {
   label = 35;
   break;
  }
 case 28:
  $p = $data + (Math_imul($y76, $bpl) | 0) | 0;
  $end = $p + (HEAP32[$width >> 2] | 0) | 0;
  label = 29;
  break;
 case 29:
  if ($p >>> 0 < $end >>> 0) {
   label = 30;
   break;
  } else {
   label = 33;
   break;
  }
 case 30:
  if (((HEAP8[$p] | 0) & 255 | 0) >= ($call75 | 0)) {
   label = 31;
   break;
  } else {
   label = 32;
   break;
  }
 case 31:
  HEAP8[$p] = 0;
  label = 32;
  break;
 case 32:
  $p = $p + 1 | 0;
  label = 29;
  break;
 case 33:
  label = 34;
  break;
 case 34:
  $y76 = $y76 + 1 | 0;
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
  STACKTOP = sp;
  return $retval | 0;
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
  return 0;
 }
 return i1 & 16384 | 0;
}
function elimOneLoopVar($argc, $argv) {
 $argc = $argc | 0;
 $argv = $argv | 0;
 var $arg$0 = 0, $call10 = Math_fround(0), $curri$012 = 0, $j$010 = 0, $ok$0 = 0, $primes$011 = 0, $retval$0 = 0, $vararg_buffer1 = 0;
 $curri$012 = 2;
 $primes$011 = 0;
 while (1) {
  $call10 = Math_fround(Math_sqrt(Math_fround(Math_fround($curri$012 | 0))));
  L15 : do {
   if ($call10 > Math_fround(+2)) {
    $j$010 = 2;
    while (1) {
     if ((($curri$012 | 0) % ($j$010 | 0) & -1 | 0) == 0) {
      $ok$0 = 0;
      break L15;
     }
     $j$010 = $j$010 + 1 | 0;
     if (!(Math_fround($j$010 | 0) < $call10)) {
      $ok$0 = 1;
      break;
     }
    }
   } else {
    $ok$0 = 1;
   }
  } while (0);
  $primes$011 = $ok$0 + $primes$011 | 0;
  if (($primes$011 | 0) >= ($arg$0 | 0)) {
   break;
  } else {
   $curri$012 = $curri$012 + 1 | 0;
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
   $storemerge3$neg9 = $26 ^ -1;
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
   $storemerge3$neg9 = $30 ^ -1;
   $26 = $30;
  }
 }
}
function elimOneLoopVar4() {
 var $storemerge3$neg9 = 0, $18 = 0, $25 = 0, $26 = 0, $jp = 0;
 $storemerge3$neg9 = -1;
 while (1) {
  $25 = $jp + ($26 << 2) | 0;
  HEAP32[$25 >> 2] = ($18 + $storemerge3$neg9 | 0) + (HEAP32[$25 >> 2] | 0) | 0;
  $26 = $26 + 1 | 0;
  if (($26 | 0) == 63) {
   break;
  } else {
   $storemerge3$neg9 = $18 ^ -1;
  }
 }
}
function elimOneLoopVarStillUsed() {
 var $call10 = Math_fround(0), $curri$012 = 0, $j$010 = 0, $retval$0 = 0;
 while (1) {
  if ((($curri$012 | 0) % ($j$010 | 0) & -1 | 0) == 0) {
   break;
  }
  $j$010 = $j$010 + 1 | 0;
  if (!(Math_fround($j$010 | 0) < $call10)) {
   break;
  }
 }
 return $retval$0 | 0;
}
function elimOneLoopVarStillUsedSE() {
 var $call10 = Math_fround(0), $curri$012 = 0, $j$010 = 0, $retval$0 = 0, $j$010$looptemp = 0;
 while (1) {
  $j$010$looptemp = $j$010;
  $j$010 = $j$010 + sideeffect() | 0;
  if ((($curri$012 | 0) % ($j$010$looptemp | 0) & -1 | 0) == 0) {
   break;
  }
  if (!(Math_fround($j$010 | 0) < $call10)) {
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
   f($30);
   break;
  } else {
   $storemerge3$neg9 = $18 ^ -1;
   $26 = $30;
  }
 }
}
function loopVarWithContinue() {
 var i = 0, i$looptemp = 0;
 i = 0;
 while (1) {
  i$looptemp = i;
  i = i + 1;
  if (check()) {
   i = i$looptemp + 1;
   continue;
  }
  work(i);
  work(i$looptemp);
  work(i);
  if (check()) {
   break;
  }
 }
}
function helperExtraUse() {
 var i = 0, i$looptemp = 0;
 i = 0;
 while (1) {
  i$looptemp = i;
  i = i + 1;
  work(i$looptemp);
  work(i);
  if (check()) {
   break;
  }
 }
 return i;
}
function mixed_up_loop_helpers() {
 var $iterations$0 = 0, $iterations$0$ph = 0, $p$sroa$0$0$ph = +0, $p$sroa$1$0$ph = +0, $p$sroa$1$0$ph$phi = +0, $vararg_buffer = 0, $vararg_ptr1 = 0, sp = 0, $iterations$0$looptemp = 0, $p$sroa$0$0$ph$looptemp = +0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 $vararg_buffer = sp;
 $iterations$0$ph = 0;
 $p$sroa$0$0$ph = +0;
 $p$sroa$1$0$ph = +1;
 L1 : while (1) {
  $iterations$0 = $iterations$0$ph;
  while (1) {
   $iterations$0$looptemp = $iterations$0;
   $iterations$0 = $iterations$0 + 1 | 0;
   if (!(($iterations$0 | 0) < 10)) {
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
   if (($iterations$0$looptemp | 0) > 0) {
    break;
   }
  }
  if (!((($iterations$0 | 0) % 3 & -1 | 0) == 0)) {
   $p$sroa$1$0$ph$phi = $p$sroa$0$0$ph;
   $iterations$0$ph = $iterations$0;
   $p$sroa$0$0$ph = -$p$sroa$1$0$ph;
   $p$sroa$1$0$ph = $p$sroa$1$0$ph$phi;
   continue;
  }
  $p$sroa$0$0$ph$looptemp = $p$sroa$0$0$ph;
  $p$sroa$0$0$ph = $p$sroa$1$0$ph * +-.5;
  $p$sroa$1$0$ph = $p$sroa$0$0$ph$looptemp * +.5;
  if ($p$sroa$0$0$ph == +0 & $p$sroa$1$0$ph == +0) {
   break;
  } else {
   $iterations$0$ph = $iterations$0;
  }
 }
}
function _postProcess() {
 cheez();
}

