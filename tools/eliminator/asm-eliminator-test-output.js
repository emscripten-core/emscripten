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
  $1 = (function() {
    try {
      __THREW__ = false;
      return __ZNSt3__16locale8__globalEv();
    } catch (e) {
      if (typeof e != "number") throw e;
      if (ABORT) throw e;
      __THREW__ = true;
      Module.print("Exception: " + e + ", currently at: " + (new Error).stack);
      return null;
    }
  })();
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
  var no = 0, yes = 0, a = 0, b = 0;
  while (1) switch (label | 0) {
   case 1:
    no = 100;
    break;
   case 2:
    yes = 111;
    yes = yes * 2;
    print(yes);
    yes--;
    print(yes / 2);
    continue;
   case 3:
    a = 5;
    b = a;
    break;
  }
}
function confuusion() {
  var i = +0, j = +0;
  func1(+i);
  j = i;
  func2(+j);
}
function tempDouble(a) {
  a = +a;
  f(a * a);
}
function _org_apache_harmony_luni_util_NumberConverter_freeFormat__($me) {
  $me = $me | 0;
  var $_r2_sroa_0 = 0, $_r3_sroa_0 = 0, $$etemp$1 = 0, $6 = 0, $7 = 0, $10 = 0, $11 = +0, $15 = 0, $_r2_sroa_0_0_cast283 = 0, $_r3_sroa_0_0_cast247 = 0, $_r3_sroa_0_0_load244 = +0, $_r3_sroa_0_0_load244$$SHADOW$0 = 0, $_r2_sroa_0_0_load = +0, $_r2_sroa_0_0_load$$SHADOW$0 = 0, $trunc297 = 0, $25 = 0, $26 = 0, $smax = 0, $28 = 0, $_r3_sroa_0_0_load239 = +0, $_pre_phi301 = 0, $_r3_sroa_0_0_cast264_pre_phi = 0, $_r2_sroa_0_0_load265 = +0, $33 = 0, $34 = 0, $_r3_sroa_0_0_cast253 = 0, $36 = 0, $37 = 0, $_r3_sroa_0_0_load243 = +0, $_r2_sroa_0_0_cast = 0, $45 = 0, $_sink_in = +0, $_r3_sroa_0_0_load241 = +0, $_r2_sroa_0_0_load266287 = 0, $_r1_sroa_0_0 = +0, $47 = 0, $48$0 = 0, $48$1 = 0, $_r1_sroa_0_0_extract_trunc185 = 0, $_r1_sroa_0_1_in$0 = 0, $_r1_sroa_0_1_in$1 = 0, $_r1_sroa_0_0_extract_trunc169 = 0, $_r1_sroa_0_2 = +0, $64 = 0, $65 = 0, $69 = 0, $76 = 0, $82 = 0, $_r1_sroa_0_0_extract_trunc = 0, $$etemp$15 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $110 = 0, $112 = 0, $113 = 0, $118 = 0, $_r3_sroa_0_0_load242 = +0, label = 0, __stackBase__ = 0;
  __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  $_r2_sroa_0 = __stackBase__ | 0;
  $_r3_sroa_0 = __stackBase__ + 8 | 0;
  if ((HEAP32[(114668 | 0) >> 2] | 0 | 0) == 0) {
    HEAP32[(114664 | 0) >> 2] = 1;
    HEAP32[(114668 | 0) >> 2] = 1;
    $$etemp$1 = 114672 | 0;
    HEAP32[($$etemp$1 | 0) >> 2] = -1;
    HEAP32[($$etemp$1 + 4 | 0) >> 2] = -1;
    HEAP32[(114684 | 0) >> 2] = 25296 | 0;
    HEAP32[(114704 | 0) >> 2] = 110728;
    HEAP32[(114712 | 0) >> 2] = 8;
    HEAP32[(114784 | 0 | 0) >> 2] = HEAP32[(107856 | 0 | 0) >> 2] | 0;
    HEAP32[(114784 | 0 | 0) + 4 >> 2] = HEAP32[(107856 | 0 | 0) + 4 >> 2] | 0;
    HEAP32[(114784 | 0 | 0) + 8 >> 2] = HEAP32[(107856 | 0 | 0) + 8 >> 2] | 0;
    HEAP32[(114784 | 0 | 0) + 12 >> 2] = HEAP32[(107856 | 0 | 0) + 12 >> 2] | 0;
    HEAP32[(114784 | 0 | 0) + 16 >> 2] = HEAP32[(107856 | 0 | 0) + 16 >> 2] | 0;
    HEAP32[(114784 | 0 | 0) + 20 >> 2] = HEAP32[(107856 | 0 | 0) + 20 >> 2] | 0;
  }
  $6 = _org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int(HEAP32[138960 >> 2] | 0, 25) | 0;
  $7 = $me + 8 | 0;
  $10 = HEAP32[($me + 20 | 0) >> 2] | 0;
  $11 = (HEAP32[tempDoublePtr >> 2] = $10, HEAP32[tempDoublePtr + 4 >> 2] = 0, +HEAPF64[tempDoublePtr >> 3]);
  if (($10 | 0) > -1) {
    HEAP32[$_r2_sroa_0 >> 2] = 0;
    $_r2_sroa_0_0_load265 = +HEAPF64[$_r2_sroa_0 >> 3];
    $_r3_sroa_0_0_cast264_pre_phi = $_r3_sroa_0;
    $_pre_phi301 = $6 + 16 | 0;
  } else {
    $15 = $6 + 16 | 0;
    HEAP16[(HEAP32[$15 >> 2] | 0) >> 1] = 48;
    $_r2_sroa_0_0_cast283 = $_r2_sroa_0;
    HEAP16[((HEAP32[$15 >> 2] | 0) + 2 | 0) >> 1] = 46;
    HEAP32[$_r2_sroa_0_0_cast283 >> 2] = 2;
    $_r3_sroa_0_0_cast247 = $_r3_sroa_0;
    HEAP32[$_r3_sroa_0_0_cast247 >> 2] = $10 + 1 | 0;
    $_r3_sroa_0_0_load244 = +HEAPF64[$_r3_sroa_0 >> 3];
    $_r3_sroa_0_0_load244$$SHADOW$0 = HEAP32[($_r3_sroa_0 | 0) >> 2] | 0;
    $_r2_sroa_0_0_load = +HEAPF64[$_r2_sroa_0 >> 3];
    $_r2_sroa_0_0_load$$SHADOW$0 = HEAP32[($_r2_sroa_0 | 0) >> 2] | 0;
    HEAPF64[$_r3_sroa_0 >> 3] = $_r2_sroa_0_0_load;
    HEAPF64[$_r2_sroa_0 >> 3] = $_r3_sroa_0_0_load244;
    $trunc297 = $_r3_sroa_0_0_load244$$SHADOW$0;
    $25 = $_r2_sroa_0_0_load$$SHADOW$0;
    if (($trunc297 | 0) < 0) {
      $26 = $trunc297 + 1 | 0;
      $smax = ($26 | 0) > 0 ? $26 : 0;
      $28 = $25 + $smax | 0;
      $113 = $25;
      $112 = $trunc297;
      while (1) {
        HEAP16[((HEAP32[$15 >> 2] | 0) + ($113 << 1) | 0) >> 1] = 48;
        $118 = $112 + 1 | 0;
        if (($118 | 0) < 0) {
          $113 = $113 + 1 | 0;
          $112 = $118;
        } else {
          break;
        }
      }
      HEAP32[$_r3_sroa_0_0_cast247 >> 2] = $28 - $trunc297 | 0;
      HEAP32[$_r2_sroa_0_0_cast283 >> 2] = $smax;
      $_r3_sroa_0_0_load239 = +HEAPF64[$_r3_sroa_0 >> 3];
    } else {
      $_r3_sroa_0_0_load239 = $_r2_sroa_0_0_load;
    }
    HEAPF64[$_r2_sroa_0 >> 3] = $_r3_sroa_0_0_load239;
    $_r2_sroa_0_0_load265 = $_r3_sroa_0_0_load239;
    $_r3_sroa_0_0_cast264_pre_phi = $_r3_sroa_0_0_cast247;
    $_pre_phi301 = $15;
  }
  $33 = $me + 16 | 0;
  $34 = HEAP32[$33 >> 2] | 0;
  $_r3_sroa_0_0_cast253 = $_r3_sroa_0;
  HEAP32[$_r3_sroa_0_0_cast253 >> 2] = $34;
  $36 = $me + 12 | 0;
  $37 = HEAP32[$36 >> 2] | 0;
  HEAP32[$36 >> 2] = $37 + 1 | 0;
  HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = HEAP32[((HEAP32[($34 + 16 | 0) >> 2] | 0) + ($37 << 2) | 0) >> 2] | 0;
  $_r3_sroa_0_0_load243 = +HEAPF64[$_r3_sroa_0 >> 3];
  HEAPF64[$_r3_sroa_0 >> 3] = $_r2_sroa_0_0_load265;
  HEAPF64[$_r2_sroa_0 >> 3] = $11;
  $_r2_sroa_0_0_cast = $_r2_sroa_0;
  $45 = $7;
  $_r1_sroa_0_0 = $_r3_sroa_0_0_load243;
  $_r2_sroa_0_0_load266287 = $10;
  $_r3_sroa_0_0_load241 = $_r2_sroa_0_0_load265;
  $_sink_in = $_r2_sroa_0_0_load265;
  while (1) {
    HEAPF64[tempDoublePtr >> 3] = $_sink_in;
    $47 = HEAP32[tempDoublePtr >> 2] | 0;
    HEAPF64[tempDoublePtr >> 3] = $_r1_sroa_0_0;
    $48$0 = HEAP32[tempDoublePtr >> 2] | 0;
    $48$1 = HEAP32[tempDoublePtr + 4 >> 2] | 0;
    $_r1_sroa_0_0_extract_trunc185 = $48$0;
    do {
      if (($_r1_sroa_0_0_extract_trunc185 | 0) == -1) {
        if (($_r2_sroa_0_0_load266287 | 0) < -1) {
          $_r1_sroa_0_2 = $_r3_sroa_0_0_load241;
          break;
        }
        HEAP16[((HEAP32[$_pre_phi301 >> 2] | 0) + ($47 << 1) | 0) >> 1] = 48;
        $_r1_sroa_0_1_in$1 = 0 | $48$1 & -1;
        $_r1_sroa_0_1_in$0 = $47 + 1 | 0 | $48$0 & 0;
        label = 785;
        break;
      } else {
        HEAP16[((HEAP32[$_pre_phi301 >> 2] | 0) + ($47 << 1) | 0) >> 1] = ($_r1_sroa_0_0_extract_trunc185 + 48 | 0) & 65535;
        $_r1_sroa_0_1_in$1 = 0;
        $_r1_sroa_0_1_in$0 = $47 + 1 | 0;
        label = 785;
        break;
      }
    } while (0);
    do {
      if ((label | 0) == 785) {
        label = 0;
        if (!(($_r2_sroa_0_0_load266287 | 0) == 0)) {
          $_r1_sroa_0_2 = (HEAP32[tempDoublePtr >> 2] = $_r1_sroa_0_1_in$0, HEAP32[tempDoublePtr + 4 >> 2] = $_r1_sroa_0_1_in$1, +HEAPF64[tempDoublePtr >> 3]);
          break;
        }
        $_r1_sroa_0_0_extract_trunc169 = $_r1_sroa_0_1_in$0;
        HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $_r1_sroa_0_0_extract_trunc169 + 1 | 0;
        HEAP16[((HEAP32[$_pre_phi301 >> 2] | 0) + ($_r1_sroa_0_0_extract_trunc169 << 1) | 0) >> 1] = 46;
        $_r1_sroa_0_2 = +HEAPF64[$_r3_sroa_0 >> 3];
      }
    } while (0);
    $64 = $_r2_sroa_0_0_load266287 - 1 | 0;
    $65 = HEAP32[$36 >> 2] | 0;
    HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $65;
    if (($65 | 0) < (HEAP32[$45 >> 2] | 0 | 0)) {
      $69 = HEAP32[$33 >> 2] | 0;
      HEAP32[$_r3_sroa_0_0_cast253 >> 2] = $69;
      HEAP32[$36 >> 2] = $65 + 1 | 0;
      $76 = HEAP32[((HEAP32[($69 + 16 | 0) >> 2] | 0) + ($65 << 2) | 0) >> 2] | 0;
      HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $76;
      if (!(($76 | 0) != -1 | ($64 | 0) > -2)) {
        break;
      }
    } else {
      HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = -1;
      if (!(($64 | 0) > -2)) {
        break;
      }
    }
    $_r3_sroa_0_0_load242 = +HEAPF64[$_r3_sroa_0 >> 3];
    HEAPF64[$_r3_sroa_0 >> 3] = $_r1_sroa_0_2;
    $_r1_sroa_0_0 = $_r3_sroa_0_0_load242;
    $_r2_sroa_0_0_load266287 = $64;
    $_r3_sroa_0_0_load241 = $_r1_sroa_0_2;
    $_sink_in = $_r1_sroa_0_2;
  }
  HEAP32[$_r2_sroa_0_0_cast >> 2] = $64;
  if ((HEAP32[(106148 | 0) >> 2] | 0 | 0) == 0) {
    ___INIT_java_lang_String();
  }
  $82 = _GC_MALLOC(36 | 0) | 0;
  HEAP32[$82 >> 2] = 106144;
  _memset($82 + 4 | 0 | 0 | 0, 0 | 0 | 0, 32 | 0 | 0);
  HEAP32[$_r2_sroa_0 >> 2] = $82;
  HEAPF64[tempDoublePtr >> 3] = $_r1_sroa_0_2;
  $_r1_sroa_0_0_extract_trunc = HEAP32[tempDoublePtr >> 2] | 0;
  HEAP32[($82 + 8 | 0) >> 2] = 0;
  HEAP32[($82 + 12 | 0) >> 2] = 0;
  HEAP32[($82 + 16 | 0) >> 2] = 0;
  if (($_r1_sroa_0_0_extract_trunc | 0) < 0) {
    _XMLVM_ERROR(16136 | 0, 13208 | 0, 132112 | 0, 830);
    return 0 | 0;
  }
  if ((HEAP32[($6 + 12 | 0) >> 2] | 0 | 0) < ($_r1_sroa_0_0_extract_trunc | 0)) {
    _XMLVM_ERROR(16136 | 0, 13208 | 0, 132112 | 0, 830);
    return 0 | 0;
  }
  HEAP32[($82 + 24 | 0) >> 2] = 0;
  if (!((HEAP32[(114668 | 0) >> 2] | 0 | 0) == 0)) {
    $105 = HEAP32[138960 >> 2] | 0;
    $106 = _org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int($105, $_r1_sroa_0_0_extract_trunc) | 0;
    $107 = $82 + 20 | 0;
    $108 = $107;
    HEAP32[$108 >> 2] = $106;
    $109 = $82 + 28 | 0;
    $110 = $109;
    HEAP32[$110 >> 2] = $_r1_sroa_0_0_extract_trunc;
    _java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int($6, 0, $106, 0, $_r1_sroa_0_0_extract_trunc);
    STACKTOP = __stackBase__;
    return $82 | 0;
  }
  HEAP32[(114664 | 0) >> 2] = 1;
  HEAP32[(114668 | 0) >> 2] = 1;
  $$etemp$15 = 114672 | 0;
  HEAP32[($$etemp$15 | 0) >> 2] = -1;
  HEAP32[($$etemp$15 + 4 | 0) >> 2] = -1;
  HEAP32[(114684 | 0) >> 2] = 25296 | 0;
  HEAP32[(114704 | 0) >> 2] = 110728;
  HEAP32[(114712 | 0) >> 2] = 8;
  HEAP32[(114784 | 0 | 0) >> 2] = HEAP32[(107856 | 0 | 0) >> 2] | 0;
  HEAP32[(114784 | 0 | 0) + 4 >> 2] = HEAP32[(107856 | 0 | 0) + 4 >> 2] | 0;
  HEAP32[(114784 | 0 | 0) + 8 >> 2] = HEAP32[(107856 | 0 | 0) + 8 >> 2] | 0;
  HEAP32[(114784 | 0 | 0) + 12 >> 2] = HEAP32[(107856 | 0 | 0) + 12 >> 2] | 0;
  HEAP32[(114784 | 0 | 0) + 16 >> 2] = HEAP32[(107856 | 0 | 0) + 16 >> 2] | 0;
  HEAP32[(114784 | 0 | 0) + 20 >> 2] = HEAP32[(107856 | 0 | 0) + 20 >> 2] | 0;
  $105 = HEAP32[138960 >> 2] | 0;
  $106 = _org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int($105, $_r1_sroa_0_0_extract_trunc) | 0;
  $107 = $82 + 20 | 0;
  $108 = $107;
  HEAP32[$108 >> 2] = $106;
  $109 = $82 + 28 | 0;
  $110 = $109;
  HEAP32[$110 >> 2] = $_r1_sroa_0_0_extract_trunc;
  _java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int($6, 0, $106, 0, $_r1_sroa_0_0_extract_trunc);
  STACKTOP = __stackBase__;
  return $82 | 0;
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

