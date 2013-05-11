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
  var $1 = (function() {
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
function confuusion() {
  var i = +0;
  func1(+i);
  var j = i; // add this var in the middle. should show up with right type later, auto-inferred from i's type
  func2(+j);
}
function tempDouble(a) {
  a = +a;
  var x = +0, y = +0;
  // CastAway can leave things like this as variables no longer needed. We need to identify that x's value has no side effects so it can be completely cleaned up
  x = (HEAP32[((tempDoublePtr)>>2)]=((HEAP32[(($_sroa_0_0__idx1)>>2)])|0),HEAP32[(((tempDoublePtr)+(4))>>2)]=((HEAP32[((($_sroa_0_0__idx1)+(4))>>2)])|0),(+(HEAPF64[(tempDoublePtr)>>3])));
  y = a*a;
  f(y);
}
function _org_apache_harmony_luni_util_NumberConverter_freeFormat__($me) {
  $me = $me | 0;
  var $_r2_sroa_0 = 0, $_r3_sroa_0 = 0, $1 = 0, $2 = 0, $$etemp$1 = 0, $$etemp$0$0 = 0, $$etemp$0$1 = 0, $st$2$0 = 0;
  var $st$3$1 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $10 = 0, $_r1_sroa_0_0_insert_ext191$0 = 0;
  var $_r1_sroa_0_0_insert_ext191$1 = 0, $11 = +0, $12 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $_r2_sroa_0_0_cast283 = 0;
  var $18 = 0, $19 = 0, $20 = 0, $21 = 0, $_r3_sroa_0_0_cast247 = 0, $_r3_sroa_0_0_load244 = +0, $ld$4$0 = 0, $_r3_sroa_0_0_load244$$SHADOW$0 = 0;
  var $ld$5$1 = 0, $_r3_sroa_0_0_load244$$SHADOW$1 = 0, $_r2_sroa_0_0_load = +0, $ld$6$0 = 0, $_r2_sroa_0_0_load$$SHADOW$0 = 0, $ld$7$1 = 0, $_r2_sroa_0_0_load$$SHADOW$1 = 0, $22$0 = 0;
  var $22$1 = 0, $trunc297$0 = 0, $trunc297 = 0, $23 = 0, $24$0 = 0, $24$1 = 0, $25$0 = 0, $25 = 0;
  var $26 = 0, $27 = 0, $smax = 0, $28 = 0, $29 = 0, $_r3_sroa_0_0_load239_pre = +0, $ld$8$0 = 0, $_r3_sroa_0_0_load239_pre$$SHADOW$0 = 0;
  var $ld$9$1 = 0, $_r3_sroa_0_0_load239_pre$$SHADOW$1 = 0, $_r3_sroa_0_0_load239 = +0, $_pre_phi301 = 0, $_r3_sroa_0_0_cast264_pre_phi = 0, $_r2_sroa_0_0_load265 = +0, $32 = 0, $33 = 0;
  var $34 = 0, $_r3_sroa_0_0_cast253 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $40 = 0;
  var $41 = 0, $42 = 0, $43 = 0, $44 = 0, $_r3_sroa_0_0_load243 = +0, $ld$10$0 = 0, $_r3_sroa_0_0_load243$$SHADOW$0 = 0, $ld$11$1 = 0;
  var $_r3_sroa_0_0_load243$$SHADOW$1 = 0, $_r2_sroa_0_0_cast = 0, $45 = 0, $_sink_in = +0, $_r3_sroa_0_0_load241 = +0, $_r2_sroa_0_0_load266287 = 0, $_r1_sroa_0_0 = +0, $_sink$0 = 0, $_sink$1 = 0;
  var $47$0 = 0, $47 = 0, $48$0 = 0, $48$1 = 0, $_r1_sroa_0_0_extract_trunc185$0 = 0, $_r1_sroa_0_0_extract_trunc185 = 0, $49 = 0, $51 = 0, $52 = 0;
  var $53 = 0, $54 = 0, $55 = 0, $56 = 0, $_r1_sroa_0_0_insert_ext195$0 = 0, $_r1_sroa_0_0_insert_ext195$1 = 0, $_r1_sroa_0_1_in$0 = 0, $_r1_sroa_0_1_in$1 = 0;
  var $_r1_sroa_0_1 = +0, $58 = 0, $_r1_sroa_0_0_extract_trunc169$0 = 0, $_r1_sroa_0_0_extract_trunc169 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0;
  var $_r3_sroa_0_0_load240 = +0, $ld$12$0 = 0, $_r3_sroa_0_0_load240$$SHADOW$0 = 0, $ld$13$1 = 0, $_r3_sroa_0_0_load240$$SHADOW$1 = 0, $_r1_sroa_0_2 = +0, $64 = 0, $65 = 0;
  var $66 = 0, $67 = 0, $69 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0;
  var $75 = 0, $76 = 0, $phitmp = 0, $77 = 0, $or_cond = 0, $79 = 0, $80 = 0, $82 = 0;
  var $83 = 0, $84 = 0, $_r2_sroa_0_0_cast269 = 0, $85$0 = 0, $85$1 = 0, $_r1_sroa_0_0_extract_trunc$0 = 0, $_r1_sroa_0_0_extract_trunc = 0, $86 = 0, $87 = 0;
  var $88 = 0, $89 = 0, $90 = 0, $91 = 0, $92 = 0, $94 = 0, $95 = 0, $96 = 0;
  var $97 = 0, $99 = 0, $100 = 0, $101 = 0, $102 = 0, $$etemp$15 = 0, $$etemp$14$0 = 0, $$etemp$14$1 = 0;
  var $st$16$0 = 0, $st$17$1 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $110 = 0;
  var $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0;
  var $121 = 0, $123 = 0, $_r1_sroa_0_0_insert_ext$0 = 0, $_r1_sroa_0_0_insert_ext$1 = 0, $$etemp$18$0 = 0, $$etemp$18$1 = 0, $_r1_sroa_0_0_insert_mask$0 = 0, $_r1_sroa_0_0_insert_mask$1 = 0;
  var $_r1_sroa_0_0_insert_insert$0 = 0, $_r1_sroa_0_0_insert_insert$1 = 0, $124 = 0, $125 = 0, $126 = 0, $_old = 0, $_r3_sroa_0_0_load242 = +0, $ld$19$0 = 0;
  var $_r3_sroa_0_0_load242$$SHADOW$0 = 0, $ld$20$1 = 0, $_r3_sroa_0_0_load242$$SHADOW$1 = 0, $_r2_sroa_0_0_cast284 = 0, $_r2_sroa_0_0_load265_pre = +0, $ld$21$0 = 0, $_r2_sroa_0_0_load265_pre$$SHADOW$0 = 0, $ld$22$1 = 0;
  var $_r2_sroa_0_0_load265_pre$$SHADOW$1 = 0, $_r3_sroa_0_0_cast264_pre = 0, $_pre = 0, $_pre300 = 0;
  var label = 0;
  var __stackBase__ = 0;
  __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  $_r2_sroa_0 = __stackBase__ | 0;
  $_r3_sroa_0 = __stackBase__ + 8 | 0;
  $1 = HEAP32[(114668 | 0) >> 2] | 0;
  $2 = ($1 | 0) == 0;
  if ($2) {
    HEAP32[(114664 | 0) >> 2] = 1;
    HEAP32[(114668 | 0) >> 2] = 1;
    $$etemp$1 = 114672 | 0;
    $$etemp$0$0 = -1;
    $$etemp$0$1 = -1;
    $st$2$0 = $$etemp$1 | 0;
    HEAP32[$st$2$0 >> 2] = $$etemp$0$0;
    $st$3$1 = $$etemp$1 + 4 | 0;
    HEAP32[$st$3$1 >> 2] = $$etemp$0$1;
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
  $5 = HEAP32[138960 >> 2] | 0;
  $6 = _org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int($5, 25) | 0;
  $7 = $me + 8 | 0;
  $8 = $me + 20 | 0;
  $9 = $8;
  $10 = HEAP32[$9 >> 2] | 0;
  $_r1_sroa_0_0_insert_ext191$0 = $10;
  $_r1_sroa_0_0_insert_ext191$1 = 0;
  $11 = (HEAP32[tempDoublePtr >> 2] = $_r1_sroa_0_0_insert_ext191$0, HEAP32[tempDoublePtr + 4 >> 2] = $_r1_sroa_0_0_insert_ext191$1, +HEAPF64[tempDoublePtr >> 3]);
  $12 = ($10 | 0) > -1;
  if ($12) {
    $_r2_sroa_0_0_cast284 = $_r2_sroa_0;
    HEAP32[$_r2_sroa_0_0_cast284 >> 2] = 0;
    $_r2_sroa_0_0_load265_pre = +HEAPF64[$_r2_sroa_0 >> 3];
    $ld$21$0 = $_r2_sroa_0 | 0;
    $_r2_sroa_0_0_load265_pre$$SHADOW$0 = HEAP32[$ld$21$0 >> 2] | 0;
    $ld$22$1 = $_r2_sroa_0 + 4 | 0;
    $_r2_sroa_0_0_load265_pre$$SHADOW$1 = HEAP32[$ld$22$1 >> 2] | 0;
    $_r3_sroa_0_0_cast264_pre = $_r3_sroa_0;
    $_pre = $6 + 16 | 0;
    $_pre300 = $_pre;
    var $_r2_sroa_0_0_load265 = $_r2_sroa_0_0_load265_pre;
    var $_r3_sroa_0_0_cast264_pre_phi = $_r3_sroa_0_0_cast264_pre;
    var $_pre_phi301 = $_pre300;
  } else {
    $14 = $6 + 16 | 0;
    $15 = $14;
    $16 = HEAP32[$15 >> 2] | 0;
    $17 = $16;
    HEAP16[$17 >> 1] = 48;
    $_r2_sroa_0_0_cast283 = $_r2_sroa_0;
    $18 = HEAP32[$15 >> 2] | 0;
    $19 = $18 + 2 | 0;
    $20 = $19;
    HEAP16[$20 >> 1] = 46;
    HEAP32[$_r2_sroa_0_0_cast283 >> 2] = 2;
    $21 = $10 + 1 | 0;
    $_r3_sroa_0_0_cast247 = $_r3_sroa_0;
    HEAP32[$_r3_sroa_0_0_cast247 >> 2] = $21;
    $_r3_sroa_0_0_load244 = +HEAPF64[$_r3_sroa_0 >> 3];
    $ld$4$0 = $_r3_sroa_0 | 0;
    $_r3_sroa_0_0_load244$$SHADOW$0 = HEAP32[$ld$4$0 >> 2] | 0;
    $ld$5$1 = $_r3_sroa_0 + 4 | 0;
    $_r3_sroa_0_0_load244$$SHADOW$1 = HEAP32[$ld$5$1 >> 2] | 0;
    $_r2_sroa_0_0_load = +HEAPF64[$_r2_sroa_0 >> 3];
    $ld$6$0 = $_r2_sroa_0 | 0;
    $_r2_sroa_0_0_load$$SHADOW$0 = HEAP32[$ld$6$0 >> 2] | 0;
    $ld$7$1 = $_r2_sroa_0 + 4 | 0;
    $_r2_sroa_0_0_load$$SHADOW$1 = HEAP32[$ld$7$1 >> 2] | 0;
    HEAPF64[$_r3_sroa_0 >> 3] = $_r2_sroa_0_0_load;
    HEAPF64[$_r2_sroa_0 >> 3] = $_r3_sroa_0_0_load244;
    $22$0 = $_r3_sroa_0_0_load244$$SHADOW$0;
    $22$1 = $_r3_sroa_0_0_load244$$SHADOW$1;
    $trunc297$0 = $22$0;
    $trunc297 = $trunc297$0;
    $23 = ($trunc297 | 0) < 0;
    $24$0 = $_r2_sroa_0_0_load$$SHADOW$0;
    $24$1 = $_r2_sroa_0_0_load$$SHADOW$1;
    $25$0 = $24$0;
    $25 = $25$0;
    if ($23) {
      $26 = $trunc297 + 1 | 0;
      $27 = ($26 | 0) > 0;
      $smax = $27 ? $26 : 0;
      $28 = $25 + $smax | 0;
      var $113 = $25;
      var $112 = $trunc297;
      while (1) {
        $114 = $113 + 1 | 0;
        $115 = HEAP32[$15 >> 2] | 0;
        $116 = $115;
        $117 = $116 + ($113 << 1) | 0;
        HEAP16[$117 >> 1] = 48;
        $118 = $112 + 1 | 0;
        $119 = ($118 | 0) < 0;
        if ($119) {
          var $113 = $114;
          var $112 = $118;
        } else {
          break;
        }
      }
      $29 = $28 - $trunc297 | 0;
      HEAP32[$_r3_sroa_0_0_cast247 >> 2] = $29;
      HEAP32[$_r2_sroa_0_0_cast283 >> 2] = $smax;
      $_r3_sroa_0_0_load239_pre = +HEAPF64[$_r3_sroa_0 >> 3];
      $ld$8$0 = $_r3_sroa_0 | 0;
      $_r3_sroa_0_0_load239_pre$$SHADOW$0 = HEAP32[$ld$8$0 >> 2] | 0;
      $ld$9$1 = $_r3_sroa_0 + 4 | 0;
      $_r3_sroa_0_0_load239_pre$$SHADOW$1 = HEAP32[$ld$9$1 >> 2] | 0;
      $_r3_sroa_0_0_load239 = $_r3_sroa_0_0_load239_pre;
    } else {
      $_r3_sroa_0_0_load239 = $_r2_sroa_0_0_load;
    }
    HEAPF64[$_r2_sroa_0 >> 3] = $_r3_sroa_0_0_load239;
    var $_r2_sroa_0_0_load265 = $_r3_sroa_0_0_load239;
    var $_r3_sroa_0_0_cast264_pre_phi = $_r3_sroa_0_0_cast247;
    var $_pre_phi301 = $15;
  }
  $32 = $me + 16 | 0;
  $33 = $32;
  $34 = HEAP32[$33 >> 2] | 0;
  $_r3_sroa_0_0_cast253 = $_r3_sroa_0;
  HEAP32[$_r3_sroa_0_0_cast253 >> 2] = $34;
  $35 = $me + 12 | 0;
  $36 = $35;
  $37 = HEAP32[$36 >> 2] | 0;
  $38 = $37 + 1 | 0;
  HEAP32[$36 >> 2] = $38;
  $39 = $34 + 16 | 0;
  $40 = $39;
  $41 = HEAP32[$40 >> 2] | 0;
  $42 = $41;
  $43 = $42 + ($37 << 2) | 0;
  $44 = HEAP32[$43 >> 2] | 0;
  HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $44;
  $_r3_sroa_0_0_load243 = +HEAPF64[$_r3_sroa_0 >> 3];
  $ld$10$0 = $_r3_sroa_0 | 0;
  $_r3_sroa_0_0_load243$$SHADOW$0 = HEAP32[$ld$10$0 >> 2] | 0;
  $ld$11$1 = $_r3_sroa_0 + 4 | 0;
  $_r3_sroa_0_0_load243$$SHADOW$1 = HEAP32[$ld$11$1 >> 2] | 0;
  HEAPF64[$_r3_sroa_0 >> 3] = $_r2_sroa_0_0_load265;
  HEAPF64[$_r2_sroa_0 >> 3] = $11;
  $_r2_sroa_0_0_cast = $_r2_sroa_0;
  $45 = $7;
  var $_r1_sroa_0_0 = $_r3_sroa_0_0_load243;
  var $_r2_sroa_0_0_load266287 = $10;
  var $_r3_sroa_0_0_load241 = $_r2_sroa_0_0_load265;
  var $_sink_in = $_r2_sroa_0_0_load265;
  while (1) {
    HEAPF64[tempDoublePtr >> 3] = $_sink_in;
    var $_sink$0 = HEAP32[tempDoublePtr >> 2] | 0;
    var $_sink$1 = HEAP32[tempDoublePtr + 4 >> 2] | 0;
    $47$0 = $_sink$0;
    $47 = $47$0;
    HEAPF64[tempDoublePtr >> 3] = $_r1_sroa_0_0;
    var $48$0 = HEAP32[tempDoublePtr >> 2] | 0;
    var $48$1 = HEAP32[tempDoublePtr + 4 >> 2] | 0;
    $_r1_sroa_0_0_extract_trunc185$0 = $48$0;
    $_r1_sroa_0_0_extract_trunc185 = $_r1_sroa_0_0_extract_trunc185$0;
    $49 = ($_r1_sroa_0_0_extract_trunc185 | 0) == -1;
    do {
      if ($49) {
        $121 = ($_r2_sroa_0_0_load266287 | 0) < -1;
        if ($121) {
          $_r1_sroa_0_2 = $_r3_sroa_0_0_load241;
          break;
        }
        $123 = $47 + 1 | 0;
        $_r1_sroa_0_0_insert_ext$0 = $123;
        $_r1_sroa_0_0_insert_ext$1 = 0;
        $$etemp$18$0 = 0;
        $$etemp$18$1 = -1;
        $_r1_sroa_0_0_insert_mask$0 = $48$0 & $$etemp$18$0;
        $_r1_sroa_0_0_insert_mask$1 = $48$1 & $$etemp$18$1;
        $_r1_sroa_0_0_insert_insert$0 = $_r1_sroa_0_0_insert_ext$0 | $_r1_sroa_0_0_insert_mask$0;
        $_r1_sroa_0_0_insert_insert$1 = $_r1_sroa_0_0_insert_ext$1 | $_r1_sroa_0_0_insert_mask$1;
        $124 = HEAP32[$_pre_phi301 >> 2] | 0;
        $125 = $124;
        $126 = $125 + ($47 << 1) | 0;
        HEAP16[$126 >> 1] = 48;
        var $_r1_sroa_0_1_in$1 = $_r1_sroa_0_0_insert_insert$1;
        var $_r1_sroa_0_1_in$0 = $_r1_sroa_0_0_insert_insert$0;
        label = 785;
        break;
      } else {
        $51 = $47 + 1 | 0;
        $52 = $_r1_sroa_0_0_extract_trunc185 + 48 | 0;
        $53 = $52 & 65535;
        $54 = HEAP32[$_pre_phi301 >> 2] | 0;
        $55 = $54;
        $56 = $55 + ($47 << 1) | 0;
        HEAP16[$56 >> 1] = $53;
        $_r1_sroa_0_0_insert_ext195$0 = $51;
        $_r1_sroa_0_0_insert_ext195$1 = 0;
        var $_r1_sroa_0_1_in$1 = $_r1_sroa_0_0_insert_ext195$1;
        var $_r1_sroa_0_1_in$0 = $_r1_sroa_0_0_insert_ext195$0;
        label = 785;
        break;
      }
    } while (0);
    do {
      if ((label | 0) == 785) {
        label = 0;
        $_r1_sroa_0_1 = (HEAP32[tempDoublePtr >> 2] = $_r1_sroa_0_1_in$0, HEAP32[tempDoublePtr + 4 >> 2] = $_r1_sroa_0_1_in$1, +HEAPF64[tempDoublePtr >> 3]);
        $58 = ($_r2_sroa_0_0_load266287 | 0) == 0;
        if (!$58) {
          $_r1_sroa_0_2 = $_r1_sroa_0_1;
          break;
        }
        $_r1_sroa_0_0_extract_trunc169$0 = $_r1_sroa_0_1_in$0;
        $_r1_sroa_0_0_extract_trunc169 = $_r1_sroa_0_0_extract_trunc169$0;
        $60 = $_r1_sroa_0_0_extract_trunc169 + 1 | 0;
        HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $60;
        $61 = HEAP32[$_pre_phi301 >> 2] | 0;
        $62 = $61;
        $63 = $62 + ($_r1_sroa_0_0_extract_trunc169 << 1) | 0;
        HEAP16[$63 >> 1] = 46;
        $_r3_sroa_0_0_load240 = +HEAPF64[$_r3_sroa_0 >> 3];
        $ld$12$0 = $_r3_sroa_0 | 0;
        $_r3_sroa_0_0_load240$$SHADOW$0 = HEAP32[$ld$12$0 >> 2] | 0;
        $ld$13$1 = $_r3_sroa_0 + 4 | 0;
        $_r3_sroa_0_0_load240$$SHADOW$1 = HEAP32[$ld$13$1 >> 2] | 0;
        $_r1_sroa_0_2 = $_r3_sroa_0_0_load240;
      }
    } while (0);
    $64 = $_r2_sroa_0_0_load266287 - 1 | 0;
    $65 = HEAP32[$36 >> 2] | 0;
    HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $65;
    $66 = HEAP32[$45 >> 2] | 0;
    $67 = ($65 | 0) < ($66 | 0);
    if ($67) {
      $69 = HEAP32[$33 >> 2] | 0;
      HEAP32[$_r3_sroa_0_0_cast253 >> 2] = $69;
      $70 = $65 + 1 | 0;
      HEAP32[$36 >> 2] = $70;
      $71 = $69 + 16 | 0;
      $72 = $71;
      $73 = HEAP32[$72 >> 2] | 0;
      $74 = $73;
      $75 = $74 + ($65 << 2) | 0;
      $76 = HEAP32[$75 >> 2] | 0;
      HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = $76;
      $phitmp = ($76 | 0) != -1;
      $77 = ($64 | 0) > -2;
      $or_cond = $phitmp | $77;
      if (!$or_cond) {
        break;
      }
    } else {
      HEAP32[$_r3_sroa_0_0_cast264_pre_phi >> 2] = -1;
      $_old = ($64 | 0) > -2;
      if (!$_old) {
        break;
      }
    }
    $_r3_sroa_0_0_load242 = +HEAPF64[$_r3_sroa_0 >> 3];
    $ld$19$0 = $_r3_sroa_0 | 0;
    $_r3_sroa_0_0_load242$$SHADOW$0 = HEAP32[$ld$19$0 >> 2] | 0;
    $ld$20$1 = $_r3_sroa_0 + 4 | 0;
    $_r3_sroa_0_0_load242$$SHADOW$1 = HEAP32[$ld$20$1 >> 2] | 0;
    HEAPF64[$_r3_sroa_0 >> 3] = $_r1_sroa_0_2;
    var $_r1_sroa_0_0 = $_r3_sroa_0_0_load242;
    var $_r2_sroa_0_0_load266287 = $64;
    var $_r3_sroa_0_0_load241 = $_r1_sroa_0_2;
    var $_sink_in = $_r1_sroa_0_2;
  }
  HEAP32[$_r2_sroa_0_0_cast >> 2] = $64;
  $79 = HEAP32[(106148 | 0) >> 2] | 0;
  $80 = ($79 | 0) == 0;
  if ($80) {
    ___INIT_java_lang_String();
  }
  $82 = _GC_MALLOC(36 | 0) | 0;
  $83 = $82;
  HEAP32[$83 >> 2] = 106144;
  $84 = $82 + 4 | 0;
  _memset($84 | 0 | 0, 0 | 0 | 0, 32 | 0 | 0);
  $_r2_sroa_0_0_cast269 = $_r2_sroa_0;
  HEAP32[$_r2_sroa_0_0_cast269 >> 2] = $82;
  HEAPF64[tempDoublePtr >> 3] = $_r1_sroa_0_2;
  var $85$0 = HEAP32[tempDoublePtr >> 2] | 0;
  var $85$1 = HEAP32[tempDoublePtr + 4 >> 2] | 0;
  $_r1_sroa_0_0_extract_trunc$0 = $85$0;
  $_r1_sroa_0_0_extract_trunc = $_r1_sroa_0_0_extract_trunc$0;
  $86 = $82 + 8 | 0;
  $87 = $86;
  HEAP32[$87 >> 2] = 0;
  $88 = $82 + 12 | 0;
  $89 = $88;
  HEAP32[$89 >> 2] = 0;
  $90 = $82 + 16 | 0;
  $91 = $90;
  HEAP32[$91 >> 2] = 0;
  $92 = ($_r1_sroa_0_0_extract_trunc | 0) < 0;
  if ($92) {
    _XMLVM_ERROR(16136 | 0, 13208 | 0, 132112 | 0, 830);
    return 0 | 0;
  }
  $94 = $6 + 12 | 0;
  $95 = $94;
  $96 = HEAP32[$95 >> 2] | 0;
  $97 = ($96 | 0) < ($_r1_sroa_0_0_extract_trunc | 0);
  if ($97) {
    _XMLVM_ERROR(16136 | 0, 13208 | 0, 132112 | 0, 830);
    return 0 | 0;
  }
  $99 = $82 + 24 | 0;
  $100 = $99;
  HEAP32[$100 >> 2] = 0;
  $101 = HEAP32[(114668 | 0) >> 2] | 0;
  $102 = ($101 | 0) == 0;
  if (!$102) {
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
  $$etemp$14$0 = -1;
  $$etemp$14$1 = -1;
  $st$16$0 = $$etemp$15 | 0;
  HEAP32[$st$16$0 >> 2] = $$etemp$14$0;
  $st$17$1 = $$etemp$15 + 4 | 0;
  HEAP32[$st$17$1 >> 2] = $$etemp$14$1;
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
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "__Z11printResultPiS_j", "_segment_holding", "__ZN5identC2EiPKcPci", "_vec2Length", "exc", "label", "confuusion", "tempDouble", "_org_apache_harmony_luni_util_NumberConverter_freeFormat__", "__ZN23b2EdgeAndPolygonContact8EvaluateEP10b2ManifoldRK11b2TransformS4_"]

