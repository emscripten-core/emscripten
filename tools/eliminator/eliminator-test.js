function a($directory) {
 var $1 = _strlen($directory);
 var $p_0 = $directory + $1 | 0;
 chak($p_0);
 var $210 = HEAP32[100]; // heaps alias each other! so this cannot be eliminated
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $210;
 chak();
 var $210a = HEAP32[100]; // function calls can also modify memory
 something();
 HEAP32[90] = $210a;
 chak();
 var $a = $hack; // no mem use (just a global), so ok to eliminate
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $a;
 chak();
 var $bb = HEAP32[11]; // ok to eliminate
 var $b = ($bb+7)|0; // ok to eliminate by itself, but not with inlined $bb which is mem-using!
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $b;
 chak();
 var $bb2 = HEAP32[11];
 HEAP32[111] = 321;
 var $b2 = ($bb2+7)|0;
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $b2;
 chak();
 var $d = HEAP32[100]; // alias on next line, but that is where we are consumed - so ok.
 HEAP32[1e3] = $d;
 chak();
 var $e = func();
 HEAP32[1e3] = $e;
 chak();
 var $e2 = func();
 tor($e2);
 chak();
 var $e3 = HEAP[9];
 tor($e3);
 barrier(); // same stuff, but with a var on top and assigns as the first and only def
 var $$210, $$210a, $$a, $$bb, $$b, $$bb2, $$b2, $$d, $$e, $$e2, $$e3;
 $$210 = HEAP32[100]; // heaps alias each other! so this cannot be eliminated
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $$210;
 chak();
 $$210a = HEAP32[100]; // function calls can also modify memory
 something();
 HEAP32[90] = $$210a;
 chak();
 $$a = $$hack; // no mem use, so ok to eliminate
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $$a;
 chak();
 $$bb = HEAP32[11]; // ok to eliminate
 $$b = ($$bb+7)|0; // ok to eliminate by itself, but not with inlined $$bb which is mem-using!
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $$b;
 chak();
 $$bb2 = HEAP32[11];
 HEAP32[111] = 321;
 $$b2 = ($$bb2+7)|0;
 HEAP32[1e3] = HEAP32[5];
 HEAP32[90] = $$b2;
 chak();
 $$d = HEAP32[100]; // alias on next line, but that is where we are consumed - so ok.
 HEAP32[1e3] = $$d;
 chak();
 $$e = func();
 HEAP32[1e3] = $$e;
 chak();
 $$e2 = func();
 tor($$e2);
 chak();
 $$e3 = HEAP[9];
 tor($$e3);
 barrier();
 var $65, $image, $51$s2, $71;
 var $66 = HEAP32[$65 >> 2];
 var $71 = $66 - _int_ceildiv(HEAP32[$image >> 2], HEAP32[$51$s2]) | 0;
 HEAP32[$65 >> 2] = _int_ceildivpow2($71, HEAP32[$51$s2 + 10]);
 barr();
 var ONCE = sheep();
 while (ONCE) {
  work();
 }
 var ONCEb = 75;
 while (ONCEb) {
  work();
 }
 var $26 = __ZL3minIiET_S0_S0_12(4096, 4096 - $16 | 0); // cannot eliminate this because the call might modify FUNCTION_TABLE
 var $27 = FUNCTION_TABLE[$22]($18, $this + ($16 + 27) | 0, $26);
 print($27);
 chak();
 var zzz = 10;
 do {
  print(zzz);
 } while (0);
 var zzz1 = 10;
 do {
  print(zzz1);
 } while (1); // cannot eliminate a do-while that is not one-time
}
function b() {
 var $148 = _sqlite3Strlen30($147);
 var $150 = HEAP32[$pExpr + 16 >> 2];
 if (($150 | 0) == 0) {
  var $156 = 0;
 } else {
  var $156 = HEAP32[$150 >> 2];
 }
 var $156;
 HEAP32[$139 + ($136 << 4) + 4 >> 2] = _sqlite3FindFunction($145, $147, $148, $156, $135, 0);
 farr();
 var $a = f1();
 var $b = f2() + $a; // this could be reordered to facilitate optimization
 f3($b);
 farr();
 var finality = cheez();
 return finality;
}
function c() {
 var x = MEM[100], y = callMe(5), z = glob; // do not eliminate vars with multiple variables, if there is a call!
 var w = x*2;
 zoom(z);
 hail(w);
 sunk(y);
 barrier();
 var x2 = MEM[100], y2 = $callMe2, z2 = glob; // no call, so ok
 var w2 = x2*2;
 zoom(z2);
 hail(w2);
 sunk(y2);
 var fly = you(fools);
 var one, two = three(); // more than one var here, so cannot eliminate |two=|, oh well
 var noneed;
 noneed = fools(you);
 var noneed2;
 noneed2 += fools(you2);
 return;
}
function f() {
 var unused;
 var x = GLOB[1];
 var y = x + 1;
 var z = y / 2;
 HEAP[123] = z;
}
function g(a1, a2) {
 var a = 1;
 var b = a * 2;
 var c = b - 1;
 var qqq = "qwe";
 a = c;
 foo(c);
 var ww = 1, www, zzz = 2;
 foo(zzz);
 for (var i = 0; i < 5; i++) {
  var q = {
   a: 1
  } + [ 2, 3 ];
 }
 for (var iterator in SOME_GLOBAL) {
  quux(iterator);
 }
 var $0 = HEAP[5];
 MAYBE_HEAP[myglobal] = 123;
 var $1 = $0 < 0;
 if ($1) {
  __label__ = 1;
 } else {
  __label__ = 2;
 }
 var sadijn = new asd;
 sadijn2 = "qwe%sert";
 this.Module || (this.Module = {});
 var obj = {
  'quoted': 1,
  "doublequoted": 2,
  unquoted: 3,
  4: 5
 };
}
function h() {
 var out;
 bar(hello);
 var hello = 5;
 if (0) {
  var sb1 = 21;
 }
 out = sb1;
 if (0) {
  var sb2 = 23;
 } else {
  out = sb2;
 }
 if (0) {
  out = sb3;
 } else {
  var sb3 = 23;
 }
 for (var it = 0; it < 5; it++) {
  x = y ? x + 1 : 7;
  var x = -5;
 }
 var oneUse = glob; // for now, cannot eliminate into body or else of if
 if (1) {
  otherGlob = oneUse;
  breakMe();
 }
 var oneUse2 = glob2;
 while (1) {
  otherGlob2 = oneUse2;
  breakMe();
 }
 return out;
}
function strtok_part(b, j, f) {
 var a;
 for (;;) {
  h = a == 13 ? h : 0;
  a = HEAP[d + h];
  if (a == g != 0) break;
  var h = h + 1;
  if (a != 0) a = 13;
 }
}
function py() {
 var $4 = HEAP[__PyThreadState_Current];
 var $5 = $4 + 12;
 var $7 = HEAP[$5] + 1;
 var $8 = $7 + 12;
 HEAP[$8] = 99;
}
var anon = function(x) {
 var $4 = HEAP[__PyThreadState_Current];
 var $5 = $4 + 12;
 var $7 = HEAP[$5] + 1;
 var $8 = $4 + 12;
 HEAP[$8] = $7;
}
function r($0) {
 HEAP[$0 + 5 + 2] = 99+5+2+1;
}
function t() {
 var $cmp2=($10) < ($11);
 if ($cmp2) { __label__ = 3; }
 var $cmp3=($12) < ($13);
 if (!($cmp3)) { __label__ = 4; }
}
function f2() {
 var $arrayidx64_phi_trans_insert = $vla + ($storemerge312 << 2) | 0;
 var $_pre = HEAPU32[$arrayidx64_phi_trans_insert >> 2];
 var $phitmp = $storemerge312 + 1 | 0;
 var $storemerge312 = $phitmp;
 var $8 = $_pre;
 c($8);
}
function f3($s, $tree, $k) {
 // HEAP vars alias each other, and the loop can confuse us
 var $0 = HEAPU32[($s + 2908 + ($k << 2) | 0) >> 2];
 while (1) {
  HEAP32[($s + 2908 + ($storemerge_in << 2) | 0) >> 2] = $9;
 }
 HEAP32[($s + 2908 + ($storemerge_in << 2) | 0) >> 2] = $0;
}
function llvm3_1() {
 while (check()) {
  var $inc = $aj_0 + 1;
  if ($curri_01 % $zj_0 == 0) {
   break;
  }
  var $j_0 = $inc;
  run($j_0 / 2);
 }
}
function _inflate($strm, $flush) {
 var __stackBase__ = STACKTOP;
 STACKTOP += 4;
 var __label__;
 var $hbuf = __stackBase__;
 var $cmp = ($strm | 0) == 0;
 $_$2 : do {
  if ($cmp) {
   var $retval_0 = -2;
  } else {
   var $state1 = $strm + 28 | 0;
   var $0 = HEAPU32[$state1 >> 2];
   var $cmp2 = ($0 | 0) == 0;
   if ($cmp2) {
    var $retval_0 = -2;
    break;
   }
   var $next_out = $strm + 12 | 0;
   var $1 = HEAP32[$next_out >> 2];
   var $cmp4 = ($1 | 0) == 0;
   if ($cmp4) {
    var $retval_0 = -2;
    break;
   }
   var $next_in = $strm | 0;
   var $2 = HEAP32[$next_in >> 2];
   var $cmp6 = ($2 | 0) == 0;
   if ($cmp6) {
    var $avail_in = $strm + 4 | 0;
    var $3 = HEAP32[$avail_in >> 2];
    var $cmp7 = ($3 | 0) == 0;
    if (!$cmp7) {
     var $retval_0 = -2;
     break;
    }
   }
   var $4 = $0;
   var $mode = $0 | 0;
   var $5 = HEAP32[$mode >> 2];
   var $cmp9 = ($5 | 0) == 11;
   if ($cmp9) {
    HEAP32[$mode >> 2] = 12;
    var $_pre = HEAP32[$next_out >> 2];
    var $_pre882 = HEAP32[$next_in >> 2];
    var $8 = $_pre;
    var $7 = $_pre882;
    var $6 = 12;
   } else {
    var $8 = $1;
    var $7 = $2;
    var $6 = $5;
   }
   var $6;
   var $7;
   var $8;
   var $avail_out = $strm + 16 | 0;
   var $9 = HEAP32[$avail_out >> 2];
   var $avail_in15 = $strm + 4 | 0;
   var $10 = HEAPU32[$avail_in15 >> 2];
   var $11 = $0 + 56 | 0;
   var $12 = HEAP32[$11 >> 2];
   var $13 = $0 + 60 | 0;
   var $14 = HEAP32[$13 >> 2];
   var $15 = $0 + 8 | 0;
   var $16 = $0 + 24 | 0;
   var $arrayidx = $hbuf | 0;
   var $arrayidx40 = $hbuf + 1 | 0;
   var $17 = $0 + 16 | 0;
   var $head = $0 + 32 | 0;
   var $18 = $head;
   var $msg = $strm + 24 | 0;
   var $19 = $0 + 36 | 0;
   var $20 = $0 + 20 | 0;
   var $adler = $strm + 48 | 0;
   var $21 = $0 + 64 | 0;
   var $22 = $0 + 12 | 0;
   var $flush_off = $flush - 5 | 0;
   var $23 = $flush_off >>> 0 < 2;
   var $24 = $0 + 4 | 0;
   var $cmp660 = ($flush | 0) == 6;
   var $25 = $0 + 7108 | 0;
   var $26 = $0 + 84 | 0;
   var $lencode1215 = $0 + 76 | 0;
   var $27 = $lencode1215;
   var $28 = $0 + 72 | 0;
   var $29 = $0 + 7112 | 0;
   var $30 = $0 + 68 | 0;
   var $31 = $0 + 44 | 0;
   var $32 = $0 + 7104 | 0;
   var $33 = $0 + 48 | 0;
   var $window = $0 + 52 | 0;
   var $34 = $window;
   var $35 = $0 + 40 | 0;
   var $total_out = $strm + 20 | 0;
   var $36 = $0 + 28 | 0;
   var $arrayidx199 = $hbuf + 2 | 0;
   var $arrayidx202 = $hbuf + 3 | 0;
   var $37 = $0 + 96 | 0;
   var $38 = $0 + 100 | 0;
   var $39 = $0 + 92 | 0;
   var $40 = $0 + 104 | 0;
   var $lens = $0 + 112 | 0;
   var $41 = $lens;
   var $codes = $0 + 1328 | 0;
   var $next861 = $0 + 108 | 0;
   var $42 = $next861;
   var $43 = $next861 | 0;
   var $arraydecay860_c = $codes;
   var $44 = $0 + 76 | 0;
   var $arraydecay864 = $lens;
   var $work = $0 + 752 | 0;
   var $arraydecay867 = $work;
   var $arrayidx1128 = $0 + 624 | 0;
   var $45 = $arrayidx1128;
   var $46 = $0 + 80 | 0;
   var $47 = $0 + 88 | 0;
   var $distcode1395 = $0 + 80 | 0;
   var $48 = $distcode1395;
   var $ret_0 = 0;
   var $next_0 = $7;
   var $put_0 = $8;
   var $have_0 = $10;
   var $left_0 = $9;
   var $hold_0 = $12;
   var $bits_0 = $14;
   var $out_0 = $9;
   var $49 = $6;
   $_$12 : while (1) {
    var $49;
    var $out_0;
    var $bits_0;
    var $hold_0;
    var $left_0;
    var $have_0;
    var $put_0;
    var $next_0;
    var $ret_0;
    $_$14 : do {
     if (($49 | 0) == 0) {
      var $50 = HEAPU32[$15 >> 2];
      var $cmp19 = ($50 | 0) == 0;
      if ($cmp19) {
       HEAP32[$mode >> 2] = 12;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_0;
       var $put_0_be = $put_0;
       var $have_0_be = $have_0;
       var $left_0_be = $left_0;
       var $hold_0_be = $hold_0;
       var $bits_0_be = $bits_0;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      var $next_1 = $next_0;
      var $have_1 = $have_0;
      var $hold_1 = $hold_0;
      var $bits_1 = $bits_0;
      while (1) {
       var $bits_1;
       var $hold_1;
       var $have_1;
       var $next_1;
       var $cmp24 = $bits_1 >>> 0 < 16;
       if (!$cmp24) {
        break;
       }
       var $cmp26 = ($have_1 | 0) == 0;
       if ($cmp26) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_1;
        var $have_58 = 0;
        var $hold_54 = $hold_1;
        var $bits_54 = $bits_1;
        var $out_4 = $out_0;
        break $_$12;
       }
       // XXX first chunk with a difference (no impact)
       var $dec = $have_1 - 1 | 0;
       var $incdec_ptr = $next_1 + 1 | 0;
       var $51 = HEAPU8[$next_1];
       var $conv = $51 & 255;
       var $shl = $conv << $bits_1;
       var $add = $shl + $hold_1 | 0;
       var $add29 = $bits_1 + 8 | 0;
       var $next_1 = $incdec_ptr;
       var $have_1 = $dec;
       var $hold_1 = $add;
       var $bits_1 = $add29;
      }
      var $and = $50 & 2;
      var $tobool = ($and | 0) != 0;
      var $cmp34 = ($hold_1 | 0) == 35615;
      var $or_cond = $tobool & $cmp34;
      if ($or_cond) {
       var $call = _crc32(0, 0, 0);
       HEAP32[$16 >> 2] = $call;
       HEAP8[$arrayidx] = 31;
       HEAP8[$arrayidx40] = -117;
       var $52 = HEAP32[$16 >> 2];
       var $call42 = _crc32($52, $arrayidx, 2);
       HEAP32[$16 >> 2] = $call42;
       HEAP32[$mode >> 2] = 1;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_1;
       var $put_0_be = $put_0;
       var $have_0_be = $have_1;
       var $left_0_be = $left_0;
       var $hold_0_be = 0;
       var $bits_0_be = 0;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      HEAP32[$17 >> 2] = 0;
      var $53 = HEAP32[$18 >> 2];
      var $cmp49 = ($53 | 0) == 0;
      if ($cmp49) {
       var $54 = $50;
      } else {
       var $done = $53 + 48 | 0;
       HEAP32[$done >> 2] = -1;
       var $_pre884 = HEAP32[$15 >> 2];
       var $54 = $_pre884;
      }
      var $54;
      var $and55 = $54 & 1;
      var $tobool56 = ($and55 | 0) == 0;
      do {
       if (!$tobool56) {
        var $and58 = $hold_1 << 8;
        var $shl59 = $and58 & 65280;
        var $shr60 = $hold_1 >>> 8;
        var $add61 = $shl59 + $shr60 | 0;
        var $rem = ($add61 >>> 0) % 31;
        var $tobool62 = ($rem | 0) == 0;
        if (!$tobool62) {
         break;
        }
        var $and66 = $hold_1 & 15;
        var $cmp67 = ($and66 | 0) == 8;
        if ($cmp67) {
         var $shr74 = $hold_1 >>> 4;
         var $sub = $bits_1 - 4 | 0;
         var $and76 = $shr74 & 15;
         var $add77 = $and76 + 8 | 0;
         var $55 = HEAPU32[$19 >> 2];
         var $cmp78 = ($55 | 0) == 0;
         do {
          if (!$cmp78) {
           var $cmp83 = $add77 >>> 0 > $55 >>> 0;
           if (!$cmp83) {
            break;
           }
           HEAP32[$msg >> 2] = STRING_TABLE.__str3100 | 0;
           HEAP32[$mode >> 2] = 29;
           var $ret_0_be = $ret_0;
           var $next_0_be = $next_1;
           var $put_0_be = $put_0;
           var $have_0_be = $have_1;
           var $left_0_be = $left_0;
           var $hold_0_be = $shr74;
           var $bits_0_be = $sub;
           var $out_0_be = $out_0;
           __label__ = 268;
           break $_$14;
          }
          HEAP32[$19 >> 2] = $add77;
         } while (0);
         var $shl90 = 1 << $add77;
         HEAP32[$20 >> 2] = $shl90;
         var $call91 = _adler32(0, 0, 0);
         HEAP32[$16 >> 2] = $call91;
         HEAP32[$adler >> 2] = $call91;
         var $and93 = $hold_1 >>> 12;
         var $56 = $and93 & 2;
         var $57 = $56 ^ 11;
         HEAP32[$mode >> 2] = $57;
         var $ret_0_be = $ret_0;
         var $next_0_be = $next_1;
         var $put_0_be = $put_0;
         var $have_0_be = $have_1;
         var $left_0_be = $left_0;
         var $hold_0_be = 0;
         var $bits_0_be = 0;
         var $out_0_be = $out_0;
         __label__ = 268;
         break $_$14;
        }
        HEAP32[$msg >> 2] = STRING_TABLE.__str299 | 0;
        HEAP32[$mode >> 2] = 29;
        var $ret_0_be = $ret_0;
        var $next_0_be = $next_1;
        var $put_0_be = $put_0;
        var $have_0_be = $have_1;
        var $left_0_be = $left_0;
        var $hold_0_be = $hold_1;
        var $bits_0_be = $bits_1;
        var $out_0_be = $out_0;
        __label__ = 268;
        break $_$14;
       }
      } while (0);
      HEAP32[$msg >> 2] = STRING_TABLE.__str198 | 0;
      HEAP32[$mode >> 2] = 29;
      var $ret_0_be = $ret_0;
      var $next_0_be = $next_1;
      var $put_0_be = $put_0;
      var $have_0_be = $have_1;
      var $left_0_be = $left_0;
      var $hold_0_be = $hold_1;
      var $bits_0_be = $bits_1;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (($49 | 0) == 1) {
      var $next_2 = $next_0;
      var $have_2 = $have_0;
      var $hold_2 = $hold_0;
      var $bits_2 = $bits_0;
      while (1) {
       var $bits_2;
       var $hold_2;
       var $have_2;
       var $next_2;
       var $cmp101 = $bits_2 >>> 0 < 16;
       if (!$cmp101) {
        break;
       }
       var $cmp105 = ($have_2 | 0) == 0;
       if ($cmp105) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_2;
        var $have_58 = 0;
        var $hold_54 = $hold_2;
        var $bits_54 = $bits_2;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec109 = $have_2 - 1 | 0;
       var $incdec_ptr110 = $next_2 + 1 | 0;
       var $58 = HEAPU8[$next_2];
       var $conv111 = $58 & 255;
       var $shl112 = $conv111 << $bits_2;
       var $add113 = $shl112 + $hold_2 | 0;
       var $add114 = $bits_2 + 8 | 0;
       var $next_2 = $incdec_ptr110;
       var $have_2 = $dec109;
       var $hold_2 = $add113;
       var $bits_2 = $add114;
      }
      HEAP32[$17 >> 2] = $hold_2;
      var $and120 = $hold_2 & 255;
      var $cmp121 = ($and120 | 0) == 8;
      if (!$cmp121) {
       HEAP32[$msg >> 2] = STRING_TABLE.__str299 | 0;
       HEAP32[$mode >> 2] = 29;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_2;
       var $put_0_be = $put_0;
       var $have_0_be = $have_2;
       var $left_0_be = $left_0;
       var $hold_0_be = $hold_2;
       var $bits_0_be = $bits_2;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      var $and128 = $hold_2 & 57344;
      var $tobool129 = ($and128 | 0) == 0;
      if ($tobool129) {
       var $59 = HEAPU32[$18 >> 2];
       var $cmp135 = ($59 | 0) == 0;
       if ($cmp135) {
        var $60 = $hold_2;
       } else {
        var $shr138 = $hold_2 >>> 8;
        var $and139 = $shr138 & 1;
        var $text = $59 | 0;
        HEAP32[$text >> 2] = $and139;
        var $_pre887 = HEAP32[$17 >> 2];
        var $60 = $_pre887;
       }
       var $60;
       var $and143 = $60 & 512;
       var $tobool144 = ($and143 | 0) == 0;
       if (!$tobool144) {
        var $conv147 = $hold_2 & 255;
        HEAP8[$arrayidx] = $conv147;
        var $shr149 = $hold_2 >>> 8;
        var $conv150 = $shr149 & 255;
        HEAP8[$arrayidx40] = $conv150;
        var $61 = HEAP32[$16 >> 2];
        var $call154 = _crc32($61, $arrayidx, 2);
        HEAP32[$16 >> 2] = $call154;
       }
       HEAP32[$mode >> 2] = 2;
       var $next_3 = $next_2;
       var $have_3 = $have_2;
       var $hold_3 = 0;
       var $bits_3 = 0;
       __label__ = 44;
       break;
      }
      HEAP32[$msg >> 2] = STRING_TABLE.__str4101 | 0;
      HEAP32[$mode >> 2] = 29;
      var $ret_0_be = $ret_0;
      var $next_0_be = $next_2;
      var $put_0_be = $put_0;
      var $have_0_be = $have_2;
      var $left_0_be = $left_0;
      var $hold_0_be = $hold_2;
      var $bits_0_be = $bits_2;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (($49 | 0) == 2) {
      var $next_3 = $next_0;
      var $have_3 = $have_0;
      var $hold_3 = $hold_0;
      var $bits_3 = $bits_0;
      __label__ = 44;
     } else if (($49 | 0) == 3) {
      var $next_4 = $next_0;
      var $have_4 = $have_0;
      var $hold_4 = $hold_0;
      var $bits_4 = $bits_0;
      __label__ = 52;
     } else if (($49 | 0) == 4) {
      var $next_5 = $next_0;
      var $have_5 = $have_0;
      var $hold_5 = $hold_0;
      var $bits_5 = $bits_0;
      __label__ = 60;
     } else if (($49 | 0) == 5) {
      var $next_8 = $next_0;
      var $have_8 = $have_0;
      var $hold_8 = $hold_0;
      var $bits_8 = $bits_0;
      __label__ = 71;
     } else if (($49 | 0) == 6) {
      var $_pre888 = HEAP32[$17 >> 2];
      var $next_11 = $next_0;
      var $have_11 = $have_0;
      var $hold_9 = $hold_0;
      var $bits_9 = $bits_0;
      var $89 = $_pre888;
      __label__ = 81;
      break;
     } else if (($49 | 0) == 7) {
      var $next_13 = $next_0;
      var $have_13 = $have_0;
      var $hold_10 = $hold_0;
      var $bits_10 = $bits_0;
      __label__ = 94;
     } else if (($49 | 0) == 8) {
      var $next_15 = $next_0;
      var $have_15 = $have_0;
      var $hold_11 = $hold_0;
      var $bits_11 = $bits_0;
      __label__ = 107;
     } else if (($49 | 0) == 9) {
      var $next_18 = $next_0;
      var $have_18 = $have_0;
      var $hold_14 = $hold_0;
      var $bits_14 = $bits_0;
      while (1) {
       var $bits_14;
       var $hold_14;
       var $have_18;
       var $next_18;
       var $cmp552 = $bits_14 >>> 0 < 32;
       if (!$cmp552) {
        break;
       }
       var $cmp556 = ($have_18 | 0) == 0;
       if ($cmp556) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_18;
        var $have_58 = 0;
        var $hold_54 = $hold_14;
        var $bits_54 = $bits_14;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec560 = $have_18 - 1 | 0;
       var $incdec_ptr561 = $next_18 + 1 | 0;
       var $114 = HEAPU8[$next_18];
       var $conv562 = $114 & 255;
       var $shl563 = $conv562 << $bits_14;
       var $add564 = $shl563 + $hold_14 | 0;
       var $add565 = $bits_14 + 8 | 0;
       var $next_18 = $incdec_ptr561;
       var $have_18 = $dec560;
       var $hold_14 = $add564;
       var $bits_14 = $add565;
      }
      var $add581 = _llvm_bswap_i32($hold_14);
      HEAP32[$16 >> 2] = $add581;
      HEAP32[$adler >> 2] = $add581;
      HEAP32[$mode >> 2] = 10;
      var $next_19 = $next_18;
      var $have_19 = $have_18;
      var $hold_15 = 0;
      var $bits_15 = 0;
      __label__ = 120;
      break;
     } else if (($49 | 0) == 10) {
      var $next_19 = $next_0;
      var $have_19 = $have_0;
      var $hold_15 = $hold_0;
      var $bits_15 = $bits_0;
      __label__ = 120;
     } else if (($49 | 0) == 11) {
      var $next_20 = $next_0;
      var $have_20 = $have_0;
      var $hold_16 = $hold_0;
      var $bits_16 = $bits_0;
      __label__ = 123;
     } else if (($49 | 0) == 12) {
      var $next_21 = $next_0;
      var $have_21 = $have_0;
      var $hold_17 = $hold_0;
      var $bits_17 = $bits_0;
      __label__ = 124;
     } else if (($49 | 0) == 13) {
      var $and681 = $bits_0 & 7;
      var $shr682 = $hold_0 >>> ($and681 >>> 0);
      var $sub684 = $bits_0 - $and681 | 0;
      var $next_23 = $next_0;
      var $have_23 = $have_0;
      var $hold_19 = $shr682;
      var $bits_19 = $sub684;
      while (1) {
       var $bits_19;
       var $hold_19;
       var $have_23;
       var $next_23;
       var $cmp689 = $bits_19 >>> 0 < 32;
       if (!$cmp689) {
        break;
       }
       var $cmp693 = ($have_23 | 0) == 0;
       if ($cmp693) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_23;
        var $have_58 = 0;
        var $hold_54 = $hold_19;
        var $bits_54 = $bits_19;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec697 = $have_23 - 1 | 0;
       var $incdec_ptr698 = $next_23 + 1 | 0;
       var $118 = HEAPU8[$next_23];
       var $conv699 = $118 & 255;
       var $shl700 = $conv699 << $bits_19;
       var $add701 = $shl700 + $hold_19 | 0;
       var $add702 = $bits_19 + 8 | 0;
       var $next_23 = $incdec_ptr698;
       var $have_23 = $dec697;
       var $hold_19 = $add701;
       var $bits_19 = $add702;
      }
      var $and708 = $hold_19 & 65535;
      var $shr709 = $hold_19 >>> 16;
      var $xor = $shr709 ^ 65535;
      var $cmp710 = ($and708 | 0) == ($xor | 0);
      if (!$cmp710) {
       HEAP32[$msg >> 2] = STRING_TABLE.__str7104 | 0;
       HEAP32[$mode >> 2] = 29;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_23;
       var $put_0_be = $put_0;
       var $have_0_be = $have_23;
       var $left_0_be = $left_0;
       var $hold_0_be = $hold_19;
       var $bits_0_be = $bits_19;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      HEAP32[$21 >> 2] = $and708;
      HEAP32[$mode >> 2] = 14;
      if ($cmp660) {
       var $ret_8 = $ret_0;
       var $next_58 = $next_23;
       var $have_58 = $have_23;
       var $hold_54 = 0;
       var $bits_54 = 0;
       var $out_4 = $out_0;
       break $_$12;
      }
      var $next_24 = $next_23;
      var $have_24 = $have_23;
      var $hold_20 = 0;
      var $bits_20 = 0;
      __label__ = 143;
      break;
     } else if (($49 | 0) == 14) {
      var $next_24 = $next_0;
      var $have_24 = $have_0;
      var $hold_20 = $hold_0;
      var $bits_20 = $bits_0;
      __label__ = 143;
     } else if (($49 | 0) == 15) {
      var $next_25 = $next_0;
      var $have_25 = $have_0;
      var $hold_21 = $hold_0;
      var $bits_21 = $bits_0;
      __label__ = 144;
     } else if (($49 | 0) == 16) {
      var $next_26 = $next_0;
      var $have_26 = $have_0;
      var $hold_22 = $hold_0;
      var $bits_22 = $bits_0;
      while (1) {
       var $bits_22;
       var $hold_22;
       var $have_26;
       var $next_26;
       var $cmp755 = $bits_22 >>> 0 < 14;
       if (!$cmp755) {
        break;
       }
       var $cmp759 = ($have_26 | 0) == 0;
       if ($cmp759) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_26;
        var $have_58 = 0;
        var $hold_54 = $hold_22;
        var $bits_54 = $bits_22;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec763 = $have_26 - 1 | 0;
       var $incdec_ptr764 = $next_26 + 1 | 0;
       var $121 = HEAPU8[$next_26];
       var $conv765 = $121 & 255;
       var $shl766 = $conv765 << $bits_22;
       var $add767 = $shl766 + $hold_22 | 0;
       var $add768 = $bits_22 + 8 | 0;
       var $next_26 = $incdec_ptr764;
       var $have_26 = $dec763;
       var $hold_22 = $add767;
       var $bits_22 = $add768;
      }
      var $and774 = $hold_22 & 31;
      var $add775 = $and774 + 257 | 0;
      HEAP32[$37 >> 2] = $add775;
      var $shr777 = $hold_22 >>> 5;
      var $and781 = $shr777 & 31;
      var $add782 = $and781 + 1 | 0;
      HEAP32[$38 >> 2] = $add782;
      var $shr784 = $hold_22 >>> 10;
      var $and788 = $shr784 & 15;
      var $add789 = $and788 + 4 | 0;
      HEAP32[$39 >> 2] = $add789;
      var $shr791 = $hold_22 >>> 14;
      var $sub792 = $bits_22 - 14 | 0;
      var $cmp796 = $add775 >>> 0 > 286;
      var $cmp800 = $add782 >>> 0 > 30;
      var $or_cond894 = $cmp796 | $cmp800;
      if ($or_cond894) {
       HEAP32[$msg >> 2] = STRING_TABLE.__str8105 | 0;
       HEAP32[$mode >> 2] = 29;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_26;
       var $put_0_be = $put_0;
       var $have_0_be = $have_26;
       var $left_0_be = $left_0;
       var $hold_0_be = $shr791;
       var $bits_0_be = $sub792;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      HEAP32[$40 >> 2] = 0;
      HEAP32[$mode >> 2] = 17;
      var $next_27 = $next_26;
      var $have_27 = $have_26;
      var $hold_23 = $shr791;
      var $bits_23 = $sub792;
      __label__ = 154;
      break;
     } else if (($49 | 0) == 17) {
      var $next_27 = $next_0;
      var $have_27 = $have_0;
      var $hold_23 = $hold_0;
      var $bits_23 = $bits_0;
      __label__ = 154;
     } else if (($49 | 0) == 18) {
      var $ret_1_ph = $ret_0;
      var $next_29_ph = $next_0;
      var $have_29_ph = $have_0;
      var $hold_25_ph = $hold_0;
      var $bits_25_ph = $bits_0;
      __label__ = 164;
     } else if (($49 | 0) == 19) {
      var $ret_2 = $ret_0;
      var $next_37 = $next_0;
      var $have_37 = $have_0;
      var $hold_33 = $hold_0;
      var $bits_33 = $bits_0;
      __label__ = 205;
     } else if (($49 | 0) == 20) {
      var $ret_3 = $ret_0;
      var $next_38 = $next_0;
      var $have_38 = $have_0;
      var $hold_34 = $hold_0;
      var $bits_34 = $bits_0;
      __label__ = 206;
     } else if (($49 | 0) == 21) {
      var $_pre889 = HEAP32[$28 >> 2];
      var $ret_4 = $ret_0;
      var $next_42 = $next_0;
      var $have_42 = $have_0;
      var $hold_38 = $hold_0;
      var $bits_38 = $bits_0;
      var $156 = $_pre889;
      __label__ = 227;
      break;
     } else if (($49 | 0) == 22) {
      var $ret_5_ph = $ret_0;
      var $next_45_ph = $next_0;
      var $have_45_ph = $have_0;
      var $hold_41_ph = $hold_0;
      var $bits_41_ph = $bits_0;
      __label__ = 234;
     } else if (($49 | 0) == 23) {
      var $_pre891 = HEAP32[$28 >> 2];
      var $ret_6 = $ret_0;
      var $next_48 = $next_0;
      var $have_48 = $have_0;
      var $hold_44 = $hold_0;
      var $bits_44 = $bits_0;
      var $167 = $_pre891;
      __label__ = 248;
      break;
     } else if (($49 | 0) == 24) {
      var $ret_7 = $ret_0;
      var $next_51 = $next_0;
      var $have_51 = $have_0;
      var $hold_47 = $hold_0;
      var $bits_47 = $bits_0;
      __label__ = 254;
     } else if (($49 | 0) == 25) {
      var $cmp1615 = ($left_0 | 0) == 0;
      if ($cmp1615) {
       var $ret_8 = $ret_0;
       var $next_58 = $next_0;
       var $have_58 = $have_0;
       var $hold_54 = $hold_0;
       var $bits_54 = $bits_0;
       var $out_4 = $out_0;
       break $_$12;
      }
      var $186 = HEAP32[$21 >> 2];
      var $conv1620 = $186 & 255;
      var $incdec_ptr1621 = $put_0 + 1 | 0;
      HEAP8[$put_0] = $conv1620;
      var $dec1622 = $left_0 - 1 | 0;
      HEAP32[$mode >> 2] = 20;
      var $ret_0_be = $ret_0;
      var $next_0_be = $next_0;
      var $put_0_be = $incdec_ptr1621;
      var $have_0_be = $have_0;
      var $left_0_be = $dec1622;
      var $hold_0_be = $hold_0;
      var $bits_0_be = $bits_0;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (($49 | 0) == 26) {
      var $187 = HEAP32[$15 >> 2];
      var $tobool1626 = ($187 | 0) == 0;
      do {
       if (!$tobool1626) {
        var $next_52 = $next_0;
        var $have_52 = $have_0;
        var $hold_48 = $hold_0;
        var $bits_48 = $bits_0;
        while (1) {
         var $bits_48;
         var $hold_48;
         var $have_52;
         var $next_52;
         var $cmp1630 = $bits_48 >>> 0 < 32;
         if (!$cmp1630) {
          break;
         }
         var $cmp1634 = ($have_52 | 0) == 0;
         if ($cmp1634) {
          var $ret_8 = $ret_0;
          var $next_58 = $next_52;
          var $have_58 = 0;
          var $hold_54 = $hold_48;
          var $bits_54 = $bits_48;
          var $out_4 = $out_0;
          break $_$12;
         }
         var $dec1638 = $have_52 - 1 | 0;
         var $incdec_ptr1639 = $next_52 + 1 | 0;
         var $188 = HEAPU8[$next_52];
         var $conv1640 = $188 & 255;
         var $shl1641 = $conv1640 << $bits_48;
         var $add1642 = $shl1641 + $hold_48 | 0;
         var $add1643 = $bits_48 + 8 | 0;
         var $next_52 = $incdec_ptr1639;
         var $have_52 = $dec1638;
         var $hold_48 = $add1642;
         var $bits_48 = $add1643;
        }
        var $sub1649 = $out_0 - $left_0 | 0;
        var $189 = HEAP32[$total_out >> 2];
        var $add1650 = $189 + $sub1649 | 0;
        HEAP32[$total_out >> 2] = $add1650;
        var $190 = HEAP32[$36 >> 2];
        var $add1651 = $190 + $sub1649 | 0;
        HEAP32[$36 >> 2] = $add1651;
        var $tobool1652 = ($out_0 | 0) == ($left_0 | 0);
        if (!$tobool1652) {
         var $191 = HEAP32[$17 >> 2];
         var $tobool1655 = ($191 | 0) == 0;
         var $192 = HEAP32[$16 >> 2];
         var $idx_neg1658 = -$sub1649 | 0;
         var $add_ptr1659 = $put_0 + $idx_neg1658 | 0;
         if ($tobool1655) {
          var $call1665 = _adler32($192, $add_ptr1659, $sub1649);
          var $cond1667 = $call1665;
         } else {
          var $call1660 = _crc32($192, $add_ptr1659, $sub1649);
          var $cond1667 = $call1660;
         }
         var $cond1667;
         HEAP32[$16 >> 2] = $cond1667;
         HEAP32[$adler >> 2] = $cond1667;
        }
        var $193 = HEAP32[$17 >> 2];
        var $tobool1672 = ($193 | 0) == 0;
        if ($tobool1672) {
         var $add1685 = _llvm_bswap_i32($hold_48);
         var $cond1687 = $add1685;
        } else {
         var $cond1687 = $hold_48;
        }
        var $cond1687;
        var $194 = HEAP32[$16 >> 2];
        var $cmp1689 = ($cond1687 | 0) == ($194 | 0);
        if ($cmp1689) {
         var $next_53 = $next_52;
         var $have_53 = $have_52;
         var $hold_49 = 0;
         var $bits_49 = 0;
         var $out_1 = $left_0;
         break;
        }
        HEAP32[$msg >> 2] = STRING_TABLE.__str17114 | 0;
        HEAP32[$mode >> 2] = 29;
        var $ret_0_be = $ret_0;
        var $next_0_be = $next_52;
        var $put_0_be = $put_0;
        var $have_0_be = $have_52;
        var $left_0_be = $left_0;
        var $hold_0_be = $hold_48;
        var $bits_0_be = $bits_48;
        var $out_0_be = $left_0;
        __label__ = 268;
        break $_$14;
       }
       var $next_53 = $next_0;
       var $have_53 = $have_0;
       var $hold_49 = $hold_0;
       var $bits_49 = $bits_0;
       var $out_1 = $out_0;
      } while (0);
      var $out_1;
      var $bits_49;
      var $hold_49;
      var $have_53;
      var $next_53;
      HEAP32[$mode >> 2] = 27;
      var $next_54 = $next_53;
      var $have_54 = $have_53;
      var $hold_50 = $hold_49;
      var $bits_50 = $bits_49;
      var $out_2 = $out_1;
      __label__ = 286;
      break;
     } else if (($49 | 0) == 27) {
      var $next_54 = $next_0;
      var $have_54 = $have_0;
      var $hold_50 = $hold_0;
      var $bits_50 = $bits_0;
      var $out_2 = $out_0;
      __label__ = 286;
     } else if (($49 | 0) == 28) {
      var $ret_8 = 1;
      var $next_58 = $next_0;
      var $have_58 = $have_0;
      var $hold_54 = $hold_0;
      var $bits_54 = $bits_0;
      var $out_4 = $out_0;
      break $_$12;
     } else if (($49 | 0) == 29) {
      var $ret_8 = -3;
      var $next_58 = $next_0;
      var $have_58 = $have_0;
      var $hold_54 = $hold_0;
      var $bits_54 = $bits_0;
      var $out_4 = $out_0;
      break $_$12;
     } else if (($49 | 0) == 30) {
      var $retval_0 = -4;
      break $_$2;
     } else {
      var $retval_0 = -2;
      break $_$2;
     }
    } while (0);
    $_$106 : do {
     if (__label__ == 44) {
      while (1) {
       var $bits_3;
       var $hold_3;
       var $have_3;
       var $next_3;
       var $cmp164 = $bits_3 >>> 0 < 32;
       if (!$cmp164) {
        break;
       }
       var $cmp168 = ($have_3 | 0) == 0;
       if ($cmp168) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_3;
        var $have_58 = 0;
        var $hold_54 = $hold_3;
        var $bits_54 = $bits_3;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec172 = $have_3 - 1 | 0;
       var $incdec_ptr173 = $next_3 + 1 | 0;
       var $62 = HEAPU8[$next_3];
       var $conv174 = $62 & 255;
       var $shl175 = $conv174 << $bits_3;
       var $add176 = $shl175 + $hold_3 | 0;
       var $add177 = $bits_3 + 8 | 0;
       var $next_3 = $incdec_ptr173;
       var $have_3 = $dec172;
       var $hold_3 = $add176;
       var $bits_3 = $add177;
      }
      var $63 = HEAP32[$18 >> 2];
      var $cmp182 = ($63 | 0) == 0;
      if (!$cmp182) {
       var $time = $63 + 4 | 0;
       HEAP32[$time >> 2] = $hold_3;
      }
      var $64 = HEAP32[$17 >> 2];
      var $and188 = $64 & 512;
      var $tobool189 = ($and188 | 0) == 0;
      if (!$tobool189) {
       var $conv192 = $hold_3 & 255;
       HEAP8[$arrayidx] = $conv192;
       var $shr194 = $hold_3 >>> 8;
       var $conv195 = $shr194 & 255;
       HEAP8[$arrayidx40] = $conv195;
       var $shr197 = $hold_3 >>> 16;
       var $conv198 = $shr197 & 255;
       HEAP8[$arrayidx199] = $conv198;
       var $shr200 = $hold_3 >>> 24;
       var $conv201 = $shr200 & 255;
       HEAP8[$arrayidx202] = $conv201;
       var $65 = HEAP32[$16 >> 2];
       var $call205 = _crc32($65, $arrayidx, 4);
       HEAP32[$16 >> 2] = $call205;
      }
      HEAP32[$mode >> 2] = 3;
      var $next_4 = $next_3;
      var $have_4 = $have_3;
      var $hold_4 = 0;
      var $bits_4 = 0;
      __label__ = 52;
      break;
     } else if (__label__ == 120) {
      var $bits_15;
      var $hold_15;
      var $have_19;
      var $next_19;
      var $115 = HEAP32[$22 >> 2];
      var $cmp589 = ($115 | 0) == 0;
      if ($cmp589) {
       HEAP32[$next_out >> 2] = $put_0;
       HEAP32[$avail_out >> 2] = $left_0;
       HEAP32[$next_in >> 2] = $next_19;
       HEAP32[$avail_in15 >> 2] = $have_19;
       HEAP32[$11 >> 2] = $hold_15;
       HEAP32[$13 >> 2] = $bits_15;
       var $retval_0 = 2;
       break $_$2;
      }
      var $call602 = _adler32(0, 0, 0);
      HEAP32[$16 >> 2] = $call602;
      HEAP32[$adler >> 2] = $call602;
      HEAP32[$mode >> 2] = 11;
      var $next_20 = $next_19;
      var $have_20 = $have_19;
      var $hold_16 = $hold_15;
      var $bits_16 = $bits_15;
      __label__ = 123;
      break;
     } else if (__label__ == 143) {
      var $bits_20;
      var $hold_20;
      var $have_24;
      var $next_24;
      HEAP32[$mode >> 2] = 15;
      var $next_25 = $next_24;
      var $have_25 = $have_24;
      var $hold_21 = $hold_20;
      var $bits_21 = $bits_20;
      __label__ = 144;
      break;
     } else if (__label__ == 154) {
      while (1) {
       var $bits_23;
       var $hold_23;
       var $have_27;
       var $next_27;
       var $122 = HEAPU32[$40 >> 2];
       var $123 = HEAPU32[$39 >> 2];
       var $cmp812 = $122 >>> 0 < $123 >>> 0;
       if (!$cmp812) {
        break;
       }
       var $next_28 = $next_27;
       var $have_28 = $have_27;
       var $hold_24 = $hold_23;
       var $bits_24 = $bits_23;
       while (1) {
        var $bits_24;
        var $hold_24;
        var $have_28;
        var $next_28;
        var $cmp817 = $bits_24 >>> 0 < 3;
        if (!$cmp817) {
         break;
        }
        var $cmp821 = ($have_28 | 0) == 0;
        if ($cmp821) {
         var $ret_8 = $ret_0;
         var $next_58 = $next_28;
         var $have_58 = 0;
         var $hold_54 = $hold_24;
         var $bits_54 = $bits_24;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $dec825 = $have_28 - 1 | 0;
        var $incdec_ptr826 = $next_28 + 1 | 0;
        var $124 = HEAPU8[$next_28];
        var $conv827 = $124 & 255;
        var $shl828 = $conv827 << $bits_24;
        var $add829 = $shl828 + $hold_24 | 0;
        var $add830 = $bits_24 + 8 | 0;
        var $next_28 = $incdec_ptr826;
        var $have_28 = $dec825;
        var $hold_24 = $add829;
        var $bits_24 = $add830;
       }
       var $hold_24_tr = $hold_24 & 65535;
       var $conv837 = $hold_24_tr & 7;
       var $inc839 = $122 + 1 | 0;
       HEAP32[$40 >> 2] = $inc839;
       var $arrayidx840 = _inflate_order + ($122 << 1) | 0;
       var $125 = HEAPU16[$arrayidx840 >> 1];
       var $idxprom = $125 & 65535;
       var $arrayidx841 = $41 + ($idxprom << 1) | 0;
       HEAP16[$arrayidx841 >> 1] = $conv837;
       var $shr843 = $hold_24 >>> 3;
       var $sub844 = $bits_24 - 3 | 0;
       var $next_27 = $next_28;
       var $have_27 = $have_28;
       var $hold_23 = $shr843;
       var $bits_23 = $sub844;
      }
      var $cmp850111 = $122 >>> 0 < 19;
      $_$131 : do {
       if ($cmp850111) {
        var $126 = $122;
        while (1) {
         var $126;
         var $inc854 = $126 + 1 | 0;
         HEAP32[$40 >> 2] = $inc854;
         var $arrayidx855 = _inflate_order + ($126 << 1) | 0;
         var $127 = HEAPU16[$arrayidx855 >> 1];
         var $idxprom856 = $127 & 65535;
         var $arrayidx858 = $41 + ($idxprom856 << 1) | 0;
         HEAP16[$arrayidx858 >> 1] = 0;
         var $_pr = HEAPU32[$40 >> 2];
         var $cmp850 = $_pr >>> 0 < 19;
         if (!$cmp850) {
          break $_$131;
         }
         var $126 = $_pr;
        }
       }
      } while (0);
      HEAP32[$43 >> 2] = $arraydecay860_c;
      HEAP32[$44 >> 2] = $arraydecay860_c;
      HEAP32[$26 >> 2] = 7;
      var $call868 = _inflate_table(0, $arraydecay864, 19, $42, $26, $arraydecay867);
      var $tobool869 = ($call868 | 0) == 0;
      if ($tobool869) {
       HEAP32[$40 >> 2] = 0;
       HEAP32[$mode >> 2] = 18;
       var $ret_1_ph = 0;
       var $next_29_ph = $next_27;
       var $have_29_ph = $have_27;
       var $hold_25_ph = $hold_23;
       var $bits_25_ph = $bits_23;
       __label__ = 164;
       break;
      }
      HEAP32[$msg >> 2] = STRING_TABLE.__str9106 | 0;
      HEAP32[$mode >> 2] = 29;
      var $ret_0_be = $call868;
      var $next_0_be = $next_27;
      var $put_0_be = $put_0;
      var $have_0_be = $have_27;
      var $left_0_be = $left_0;
      var $hold_0_be = $hold_23;
      var $bits_0_be = $bits_23;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (__label__ == 286) {
      var $out_2;
      var $bits_50;
      var $hold_50;
      var $have_54;
      var $next_54;
      var $195 = HEAP32[$15 >> 2];
      var $tobool1702 = ($195 | 0) == 0;
      do {
       if (!$tobool1702) {
        var $196 = HEAP32[$17 >> 2];
        var $tobool1705 = ($196 | 0) == 0;
        if ($tobool1705) {
         var $next_56 = $next_54;
         var $have_56 = $have_54;
         var $hold_52 = $hold_50;
         var $bits_52 = $bits_50;
         break;
        }
        var $next_55 = $next_54;
        var $have_55 = $have_54;
        var $hold_51 = $hold_50;
        var $bits_51 = $bits_50;
        while (1) {
         var $bits_51;
         var $hold_51;
         var $have_55;
         var $next_55;
         var $cmp1709 = $bits_51 >>> 0 < 32;
         if (!$cmp1709) {
          break;
         }
         var $cmp1713 = ($have_55 | 0) == 0;
         if ($cmp1713) {
          var $ret_8 = $ret_0;
          var $next_58 = $next_55;
          var $have_58 = 0;
          var $hold_54 = $hold_51;
          var $bits_54 = $bits_51;
          var $out_4 = $out_2;
          break $_$12;
         }
         var $dec1717 = $have_55 - 1 | 0;
         var $incdec_ptr1718 = $next_55 + 1 | 0;
         var $197 = HEAPU8[$next_55];
         var $conv1719 = $197 & 255;
         var $shl1720 = $conv1719 << $bits_51;
         var $add1721 = $shl1720 + $hold_51 | 0;
         var $add1722 = $bits_51 + 8 | 0;
         var $next_55 = $incdec_ptr1718;
         var $have_55 = $dec1717;
         var $hold_51 = $add1721;
         var $bits_51 = $add1722;
        }
        var $198 = HEAP32[$36 >> 2];
        var $cmp1729 = ($hold_51 | 0) == ($198 | 0);
        if ($cmp1729) {
         var $next_56 = $next_55;
         var $have_56 = $have_55;
         var $hold_52 = 0;
         var $bits_52 = 0;
         break;
        }
        HEAP32[$msg >> 2] = STRING_TABLE.__str18115 | 0;
        HEAP32[$mode >> 2] = 29;
        var $ret_0_be = $ret_0;
        var $next_0_be = $next_55;
        var $put_0_be = $put_0;
        var $have_0_be = $have_55;
        var $left_0_be = $left_0;
        var $hold_0_be = $hold_51;
        var $bits_0_be = $bits_51;
        var $out_0_be = $out_2;
        __label__ = 268;
        break $_$106;
       }
       var $next_56 = $next_54;
       var $have_56 = $have_54;
       var $hold_52 = $hold_50;
       var $bits_52 = $bits_50;
      } while (0);
      var $bits_52;
      var $hold_52;
      var $have_56;
      var $next_56;
      HEAP32[$mode >> 2] = 28;
      var $ret_8 = 1;
      var $next_58 = $next_56;
      var $have_58 = $have_56;
      var $hold_54 = $hold_52;
      var $bits_54 = $bits_52;
      var $out_4 = $out_2;
      break $_$12;
     }
    } while (0);
    $_$148 : do {
     if (__label__ == 52) {
      while (1) {
       var $bits_4;
       var $hold_4;
       var $have_4;
       var $next_4;
       var $cmp215 = $bits_4 >>> 0 < 16;
       if (!$cmp215) {
        break;
       }
       var $cmp219 = ($have_4 | 0) == 0;
       if ($cmp219) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_4;
        var $have_58 = 0;
        var $hold_54 = $hold_4;
        var $bits_54 = $bits_4;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec223 = $have_4 - 1 | 0;
       var $incdec_ptr224 = $next_4 + 1 | 0;
       var $66 = HEAPU8[$next_4];
       var $conv225 = $66 & 255;
       var $shl226 = $conv225 << $bits_4;
       var $add227 = $shl226 + $hold_4 | 0;
       var $add228 = $bits_4 + 8 | 0;
       var $next_4 = $incdec_ptr224;
       var $have_4 = $dec223;
       var $hold_4 = $add227;
       var $bits_4 = $add228;
      }
      var $67 = HEAP32[$18 >> 2];
      var $cmp233 = ($67 | 0) == 0;
      if (!$cmp233) {
       var $and236 = $hold_4 & 255;
       var $xflags = $67 + 8 | 0;
       HEAP32[$xflags >> 2] = $and236;
       var $shr238 = $hold_4 >>> 8;
       var $68 = HEAP32[$18 >> 2];
       var $os = $68 + 12 | 0;
       HEAP32[$os >> 2] = $shr238;
      }
      var $69 = HEAP32[$17 >> 2];
      var $and242 = $69 & 512;
      var $tobool243 = ($and242 | 0) == 0;
      if (!$tobool243) {
       var $conv246 = $hold_4 & 255;
       HEAP8[$arrayidx] = $conv246;
       var $shr248 = $hold_4 >>> 8;
       var $conv249 = $shr248 & 255;
       HEAP8[$arrayidx40] = $conv249;
       var $70 = HEAP32[$16 >> 2];
       var $call253 = _crc32($70, $arrayidx, 2);
       HEAP32[$16 >> 2] = $call253;
      }
      HEAP32[$mode >> 2] = 4;
      var $next_5 = $next_4;
      var $have_5 = $have_4;
      var $hold_5 = 0;
      var $bits_5 = 0;
      __label__ = 60;
      break;
     } else if (__label__ == 123) {
      var $bits_16;
      var $hold_16;
      var $have_20;
      var $next_20;
      if ($23) {
       var $ret_8 = $ret_0;
       var $next_58 = $next_20;
       var $have_58 = $have_20;
       var $hold_54 = $hold_16;
       var $bits_54 = $bits_16;
       var $out_4 = $out_0;
       break $_$12;
      }
      var $next_21 = $next_20;
      var $have_21 = $have_20;
      var $hold_17 = $hold_16;
      var $bits_17 = $bits_16;
      __label__ = 124;
      break;
     } else if (__label__ == 144) {
      var $bits_21;
      var $hold_21;
      var $have_25;
      var $next_25;
      var $119 = HEAPU32[$21 >> 2];
      var $tobool730 = ($119 | 0) == 0;
      if ($tobool730) {
       HEAP32[$mode >> 2] = 11;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_25;
       var $put_0_be = $put_0;
       var $have_0_be = $have_25;
       var $left_0_be = $left_0;
       var $hold_0_be = $hold_21;
       var $bits_0_be = $bits_21;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      var $cmp732 = $119 >>> 0 > $have_25 >>> 0;
      var $copy_3 = $cmp732 ? $have_25 : $119;
      var $cmp736 = $copy_3 >>> 0 > $left_0 >>> 0;
      var $copy_4 = $cmp736 ? $left_0 : $copy_3;
      var $cmp740 = ($copy_4 | 0) == 0;
      if ($cmp740) {
       var $ret_8 = $ret_0;
       var $next_58 = $next_25;
       var $have_58 = $have_25;
       var $hold_54 = $hold_21;
       var $bits_54 = $bits_21;
       var $out_4 = $out_0;
       break $_$12;
      }
      _memcpy($put_0, $next_25, $copy_4, 1);
      var $sub744 = $have_25 - $copy_4 | 0;
      var $add_ptr745 = $next_25 + $copy_4 | 0;
      var $sub746 = $left_0 - $copy_4 | 0;
      var $add_ptr747 = $put_0 + $copy_4 | 0;
      var $120 = HEAP32[$21 >> 2];
      var $sub749 = $120 - $copy_4 | 0;
      HEAP32[$21 >> 2] = $sub749;
      var $ret_0_be = $ret_0;
      var $next_0_be = $add_ptr745;
      var $put_0_be = $add_ptr747;
      var $have_0_be = $sub744;
      var $left_0_be = $sub746;
      var $hold_0_be = $hold_21;
      var $bits_0_be = $bits_21;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (__label__ == 164) {
      var $bits_25_ph;
      var $hold_25_ph;
      var $have_29_ph;
      var $next_29_ph;
      var $ret_1_ph;
      var $next_29 = $next_29_ph;
      var $have_29 = $have_29_ph;
      var $hold_25 = $hold_25_ph;
      var $bits_25 = $bits_25_ph;
      $_$167 : while (1) {
       var $bits_25;
       var $hold_25;
       var $have_29;
       var $next_29;
       var $128 = HEAPU32[$40 >> 2];
       var $129 = HEAPU32[$37 >> 2];
       var $130 = HEAP32[$38 >> 2];
       var $add881 = $130 + $129 | 0;
       var $cmp882 = $128 >>> 0 < $add881 >>> 0;
       if ($cmp882) {
        var $131 = HEAP32[$26 >> 2];
        var $shl887 = 1 << $131;
        var $sub888 = $shl887 - 1 | 0;
        var $132 = HEAPU32[$27 >> 2];
        var $next_30 = $next_29;
        var $have_30 = $have_29;
        var $hold_26 = $hold_25;
        var $bits_26 = $bits_25;
        while (1) {
         var $bits_26;
         var $hold_26;
         var $have_30;
         var $next_30;
         var $and889 = $sub888 & $hold_26;
         var $arrayidx891_1 = $132 + ($and889 << 2) + 1 | 0;
         var $tmp25 = HEAPU8[$arrayidx891_1];
         var $conv893 = $tmp25 & 255;
         var $cmp894 = $conv893 >>> 0 > $bits_26 >>> 0;
         if (!$cmp894) {
          break;
         }
         var $cmp899 = ($have_30 | 0) == 0;
         if ($cmp899) {
          var $ret_8 = $ret_1_ph;
          var $next_58 = $next_30;
          var $have_58 = 0;
          var $hold_54 = $hold_26;
          var $bits_54 = $bits_26;
          var $out_4 = $out_0;
          break $_$12;
         }
         var $dec903 = $have_30 - 1 | 0;
         var $incdec_ptr904 = $next_30 + 1 | 0;
         var $133 = HEAPU8[$next_30];
         var $conv905 = $133 & 255;
         var $shl906 = $conv905 << $bits_26;
         var $add907 = $shl906 + $hold_26 | 0;
         var $add908 = $bits_26 + 8 | 0;
         var $next_30 = $incdec_ptr904;
         var $have_30 = $dec903;
         var $hold_26 = $add907;
         var $bits_26 = $add908;
        }
        var $arrayidx891_2 = $132 + ($and889 << 2) + 2 | 0;
        var $tmp26 = HEAPU16[$arrayidx891_2 >> 1];
        var $cmp912 = ($tmp26 & 65535) < 16;
        if ($cmp912) {
         var $next_31 = $next_30;
         var $have_31 = $have_30;
         var $hold_27 = $hold_26;
         var $bits_27 = $bits_26;
         while (1) {
          var $bits_27;
          var $hold_27;
          var $have_31;
          var $next_31;
          var $cmp919 = $bits_27 >>> 0 < $conv893 >>> 0;
          if (!$cmp919) {
           break;
          }
          var $cmp923 = ($have_31 | 0) == 0;
          if ($cmp923) {
           var $ret_8 = $ret_1_ph;
           var $next_58 = $next_31;
           var $have_58 = 0;
           var $hold_54 = $hold_27;
           var $bits_54 = $bits_27;
           var $out_4 = $out_0;
           break $_$12;
          }
          var $dec927 = $have_31 - 1 | 0;
          var $incdec_ptr928 = $next_31 + 1 | 0;
          var $134 = HEAPU8[$next_31];
          var $conv929 = $134 & 255;
          var $shl930 = $conv929 << $bits_27;
          var $add931 = $shl930 + $hold_27 | 0;
          var $add932 = $bits_27 + 8 | 0;
          var $next_31 = $incdec_ptr928;
          var $have_31 = $dec927;
          var $hold_27 = $add931;
          var $bits_27 = $add932;
         }
         var $shr941 = $hold_27 >>> ($conv893 >>> 0);
         var $sub944 = $bits_27 - $conv893 | 0;
         var $inc949 = $128 + 1 | 0;
         HEAP32[$40 >> 2] = $inc949;
         var $arrayidx951 = $41 + ($128 << 1) | 0;
         HEAP16[$arrayidx951 >> 1] = $tmp26;
         var $next_29 = $next_31;
         var $have_29 = $have_31;
         var $hold_25 = $shr941;
         var $bits_25 = $sub944;
        } else {
         if ($tmp26 << 16 >> 16 == 16) {
          var $add962 = $conv893 + 2 | 0;
          var $next_32 = $next_30;
          var $have_32 = $have_30;
          var $hold_28 = $hold_26;
          var $bits_28 = $bits_26;
          while (1) {
           var $bits_28;
           var $hold_28;
           var $have_32;
           var $next_32;
           var $cmp963 = $bits_28 >>> 0 < $add962 >>> 0;
           if (!$cmp963) {
            break;
           }
           var $cmp967 = ($have_32 | 0) == 0;
           if ($cmp967) {
            var $ret_8 = $ret_1_ph;
            var $next_58 = $next_32;
            var $have_58 = 0;
            var $hold_54 = $hold_28;
            var $bits_54 = $bits_28;
            var $out_4 = $out_0;
            break $_$12;
           }
           var $dec971 = $have_32 - 1 | 0;
           var $incdec_ptr972 = $next_32 + 1 | 0;
           var $135 = HEAPU8[$next_32];
           var $conv973 = $135 & 255;
           var $shl974 = $conv973 << $bits_28;
           var $add975 = $shl974 + $hold_28 | 0;
           var $add976 = $bits_28 + 8 | 0;
           var $next_32 = $incdec_ptr972;
           var $have_32 = $dec971;
           var $hold_28 = $add975;
           var $bits_28 = $add976;
          }
          var $shr985 = $hold_28 >>> ($conv893 >>> 0);
          var $sub988 = $bits_28 - $conv893 | 0;
          var $cmp992 = ($128 | 0) == 0;
          if ($cmp992) {
           HEAP32[$msg >> 2] = STRING_TABLE.__str10107 | 0;
           HEAP32[$mode >> 2] = 29;
           var $ret_0_be = $ret_1_ph;
           var $next_0_be = $next_32;
           var $put_0_be = $put_0;
           var $have_0_be = $have_32;
           var $left_0_be = $left_0;
           var $hold_0_be = $shr985;
           var $bits_0_be = $sub988;
           var $out_0_be = $out_0;
           __label__ = 268;
           break $_$148;
          }
          var $sub999 = $128 - 1 | 0;
          var $arrayidx1001 = $41 + ($sub999 << 1) | 0;
          var $136 = HEAP16[$arrayidx1001 >> 1];
          var $and1003 = $shr985 & 3;
          var $add1004 = $and1003 + 3 | 0;
          var $shr1006 = $shr985 >>> 2;
          var $sub1007 = $sub988 - 2 | 0;
          var $len_0 = $136;
          var $next_35 = $next_32;
          var $have_35 = $have_32;
          var $hold_31 = $shr1006;
          var $bits_31 = $sub1007;
          var $copy_5 = $add1004;
         } else if ($tmp26 << 16 >> 16 == 17) {
          var $add1020 = $conv893 + 3 | 0;
          var $next_33 = $next_30;
          var $have_33 = $have_30;
          var $hold_29 = $hold_26;
          var $bits_29 = $bits_26;
          while (1) {
           var $bits_29;
           var $hold_29;
           var $have_33;
           var $next_33;
           var $cmp1021 = $bits_29 >>> 0 < $add1020 >>> 0;
           if (!$cmp1021) {
            break;
           }
           var $cmp1025 = ($have_33 | 0) == 0;
           if ($cmp1025) {
            var $ret_8 = $ret_1_ph;
            var $next_58 = $next_33;
            var $have_58 = 0;
            var $hold_54 = $hold_29;
            var $bits_54 = $bits_29;
            var $out_4 = $out_0;
            break $_$12;
           }
           var $dec1029 = $have_33 - 1 | 0;
           var $incdec_ptr1030 = $next_33 + 1 | 0;
           var $137 = HEAPU8[$next_33];
           var $conv1031 = $137 & 255;
           var $shl1032 = $conv1031 << $bits_29;
           var $add1033 = $shl1032 + $hold_29 | 0;
           var $add1034 = $bits_29 + 8 | 0;
           var $next_33 = $incdec_ptr1030;
           var $have_33 = $dec1029;
           var $hold_29 = $add1033;
           var $bits_29 = $add1034;
          }
          var $shr1043 = $hold_29 >>> ($conv893 >>> 0);
          var $and1049 = $shr1043 & 7;
          var $add1050 = $and1049 + 3 | 0;
          var $shr1052 = $shr1043 >>> 3;
          var $sub1046 = -3 - $conv893 | 0;
          var $sub1053 = $sub1046 + $bits_29 | 0;
          var $len_0 = 0;
          var $next_35 = $next_33;
          var $have_35 = $have_33;
          var $hold_31 = $shr1052;
          var $bits_31 = $sub1053;
          var $copy_5 = $add1050;
         } else {
          var $add1061 = $conv893 + 7 | 0;
          var $next_34 = $next_30;
          var $have_34 = $have_30;
          var $hold_30 = $hold_26;
          var $bits_30 = $bits_26;
          while (1) {
           var $bits_30;
           var $hold_30;
           var $have_34;
           var $next_34;
           var $cmp1062 = $bits_30 >>> 0 < $add1061 >>> 0;
           if (!$cmp1062) {
            break;
           }
           var $cmp1066 = ($have_34 | 0) == 0;
           if ($cmp1066) {
            var $ret_8 = $ret_1_ph;
            var $next_58 = $next_34;
            var $have_58 = 0;
            var $hold_54 = $hold_30;
            var $bits_54 = $bits_30;
            var $out_4 = $out_0;
            break $_$12;
           }
           var $dec1070 = $have_34 - 1 | 0;
           var $incdec_ptr1071 = $next_34 + 1 | 0;
           var $138 = HEAPU8[$next_34];
           var $conv1072 = $138 & 255;
           var $shl1073 = $conv1072 << $bits_30;
           var $add1074 = $shl1073 + $hold_30 | 0;
           var $add1075 = $bits_30 + 8 | 0;
           var $next_34 = $incdec_ptr1071;
           var $have_34 = $dec1070;
           var $hold_30 = $add1074;
           var $bits_30 = $add1075;
          }
          var $shr1084 = $hold_30 >>> ($conv893 >>> 0);
          var $and1090 = $shr1084 & 127;
          var $add1091 = $and1090 + 11 | 0;
          var $shr1093 = $shr1084 >>> 7;
          var $sub1087 = -7 - $conv893 | 0;
          var $sub1094 = $sub1087 + $bits_30 | 0;
          var $len_0 = 0;
          var $next_35 = $next_34;
          var $have_35 = $have_34;
          var $hold_31 = $shr1093;
          var $bits_31 = $sub1094;
          var $copy_5 = $add1091;
         }
         var $copy_5;
         var $bits_31;
         var $hold_31;
         var $have_35;
         var $next_35;
         var $len_0;
         var $add1100 = $128 + $copy_5 | 0;
         var $cmp1104 = $add1100 >>> 0 > $add881 >>> 0;
         if ($cmp1104) {
          HEAP32[$msg >> 2] = STRING_TABLE.__str10107 | 0;
          HEAP32[$mode >> 2] = 29;
          var $ret_0_be = $ret_1_ph;
          var $next_0_be = $next_35;
          var $put_0_be = $put_0;
          var $have_0_be = $have_35;
          var $left_0_be = $left_0;
          var $hold_0_be = $hold_31;
          var $bits_0_be = $bits_31;
          var $out_0_be = $out_0;
          __label__ = 268;
          break $_$148;
         }
         var $copy_6127 = $copy_5;
         var $139 = $128;
         while (1) {
          var $139;
          var $copy_6127;
          var $dec1111 = $copy_6127 - 1 | 0;
          var $inc1116 = $139 + 1 | 0;
          HEAP32[$40 >> 2] = $inc1116;
          var $arrayidx1118 = $41 + ($139 << 1) | 0;
          HEAP16[$arrayidx1118 >> 1] = $len_0;
          var $tobool1112 = ($dec1111 | 0) == 0;
          if ($tobool1112) {
           var $next_29 = $next_35;
           var $have_29 = $have_35;
           var $hold_25 = $hold_31;
           var $bits_25 = $bits_31;
           continue $_$167;
          }
          var $_pre892 = HEAP32[$40 >> 2];
          var $copy_6127 = $dec1111;
          var $139 = $_pre892;
         }
        }
       } else {
        var $_pr38 = HEAP32[$mode >> 2];
        var $cmp1123 = ($_pr38 | 0) == 29;
        if ($cmp1123) {
         var $ret_0_be = $ret_1_ph;
         var $next_0_be = $next_29;
         var $put_0_be = $put_0;
         var $have_0_be = $have_29;
         var $left_0_be = $left_0;
         var $hold_0_be = $hold_25;
         var $bits_0_be = $bits_25;
         var $out_0_be = $out_0;
         __label__ = 268;
         break $_$148;
        }
        var $140 = HEAP16[$45 >> 1];
        var $cmp1130 = $140 << 16 >> 16 == 0;
        if ($cmp1130) {
         HEAP32[$msg >> 2] = STRING_TABLE.__str11108 | 0;
         HEAP32[$mode >> 2] = 29;
         var $ret_0_be = $ret_1_ph;
         var $next_0_be = $next_29;
         var $put_0_be = $put_0;
         var $have_0_be = $have_29;
         var $left_0_be = $left_0;
         var $hold_0_be = $hold_25;
         var $bits_0_be = $bits_25;
         var $out_0_be = $out_0;
         __label__ = 268;
         break $_$148;
        }
        HEAP32[$43 >> 2] = $arraydecay860_c;
        HEAP32[$44 >> 2] = $arraydecay860_c;
        HEAP32[$26 >> 2] = 9;
        var $call1149 = _inflate_table(1, $arraydecay864, $129, $42, $26, $arraydecay867);
        var $tobool1150 = ($call1149 | 0) == 0;
        if (!$tobool1150) {
         HEAP32[$msg >> 2] = STRING_TABLE.__str12109 | 0;
         HEAP32[$mode >> 2] = 29;
         var $ret_0_be = $call1149;
         var $next_0_be = $next_29;
         var $put_0_be = $put_0;
         var $have_0_be = $have_29;
         var $left_0_be = $left_0;
         var $hold_0_be = $hold_25;
         var $bits_0_be = $bits_25;
         var $out_0_be = $out_0;
         __label__ = 268;
         break $_$148;
        }
        var $141 = HEAP32[$42 >> 2];
        var $_c = $141;
        HEAP32[$46 >> 2] = $_c;
        HEAP32[$47 >> 2] = 6;
        var $142 = HEAP32[$37 >> 2];
        var $add_ptr1159 = $arraydecay864 + ($142 << 1) | 0;
        var $143 = HEAP32[$38 >> 2];
        var $call1165 = _inflate_table(2, $add_ptr1159, $143, $42, $47, $arraydecay867);
        var $tobool1166 = ($call1165 | 0) == 0;
        if (!$tobool1166) {
         HEAP32[$msg >> 2] = STRING_TABLE.__str13110 | 0;
         HEAP32[$mode >> 2] = 29;
         var $ret_0_be = $call1165;
         var $next_0_be = $next_29;
         var $put_0_be = $put_0;
         var $have_0_be = $have_29;
         var $left_0_be = $left_0;
         var $hold_0_be = $hold_25;
         var $bits_0_be = $bits_25;
         var $out_0_be = $out_0;
         __label__ = 268;
         break $_$148;
        }
        HEAP32[$mode >> 2] = 19;
        if ($cmp660) {
         var $ret_8 = 0;
         var $next_58 = $next_29;
         var $have_58 = $have_29;
         var $hold_54 = $hold_25;
         var $bits_54 = $bits_25;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $ret_2 = 0;
        var $next_37 = $next_29;
        var $have_37 = $have_29;
        var $hold_33 = $hold_25;
        var $bits_33 = $bits_25;
        __label__ = 205;
        break $_$148;
       }
      }
     }
    } while (0);
    do {
     if (__label__ == 60) {
      var $bits_5;
      var $hold_5;
      var $have_5;
      var $next_5;
      var $71 = HEAPU32[$17 >> 2];
      var $and262 = $71 & 1024;
      var $tobool263 = ($and262 | 0) == 0;
      do {
       if ($tobool263) {
        var $76 = HEAP32[$18 >> 2];
        var $cmp310 = ($76 | 0) == 0;
        if ($cmp310) {
         var $next_7 = $next_5;
         var $have_7 = $have_5;
         var $hold_7 = $hold_5;
         var $bits_7 = $bits_5;
         break;
        }
        var $extra = $76 + 16 | 0;
        HEAP32[$extra >> 2] = 0;
        var $next_7 = $next_5;
        var $have_7 = $have_5;
        var $hold_7 = $hold_5;
        var $bits_7 = $bits_5;
       } else {
        var $next_6 = $next_5;
        var $have_6 = $have_5;
        var $hold_6 = $hold_5;
        var $bits_6 = $bits_5;
        while (1) {
         var $bits_6;
         var $hold_6;
         var $have_6;
         var $next_6;
         var $cmp267 = $bits_6 >>> 0 < 16;
         if (!$cmp267) {
          break;
         }
         var $cmp271 = ($have_6 | 0) == 0;
         if ($cmp271) {
          var $ret_8 = $ret_0;
          var $next_58 = $next_6;
          var $have_58 = 0;
          var $hold_54 = $hold_6;
          var $bits_54 = $bits_6;
          var $out_4 = $out_0;
          break $_$12;
         }
         var $dec275 = $have_6 - 1 | 0;
         var $incdec_ptr276 = $next_6 + 1 | 0;
         var $72 = HEAPU8[$next_6];
         var $conv277 = $72 & 255;
         var $shl278 = $conv277 << $bits_6;
         var $add279 = $shl278 + $hold_6 | 0;
         var $add280 = $bits_6 + 8 | 0;
         var $next_6 = $incdec_ptr276;
         var $have_6 = $dec275;
         var $hold_6 = $add279;
         var $bits_6 = $add280;
        }
        HEAP32[$21 >> 2] = $hold_6;
        var $73 = HEAP32[$18 >> 2];
        var $cmp285 = ($73 | 0) == 0;
        if ($cmp285) {
         var $74 = $71;
        } else {
         var $extra_len = $73 + 20 | 0;
         HEAP32[$extra_len >> 2] = $hold_6;
         var $_pre885 = HEAP32[$17 >> 2];
         var $74 = $_pre885;
        }
        var $74;
        var $and291 = $74 & 512;
        var $tobool292 = ($and291 | 0) == 0;
        if ($tobool292) {
         var $next_7 = $next_6;
         var $have_7 = $have_6;
         var $hold_7 = 0;
         var $bits_7 = 0;
         break;
        }
        var $conv295 = $hold_6 & 255;
        HEAP8[$arrayidx] = $conv295;
        var $shr297 = $hold_6 >>> 8;
        var $conv298 = $shr297 & 255;
        HEAP8[$arrayidx40] = $conv298;
        var $75 = HEAP32[$16 >> 2];
        var $call302 = _crc32($75, $arrayidx, 2);
        HEAP32[$16 >> 2] = $call302;
        var $next_7 = $next_6;
        var $have_7 = $have_6;
        var $hold_7 = 0;
        var $bits_7 = 0;
       }
      } while (0);
      var $bits_7;
      var $hold_7;
      var $have_7;
      var $next_7;
      HEAP32[$mode >> 2] = 5;
      var $next_8 = $next_7;
      var $have_8 = $have_7;
      var $hold_8 = $hold_7;
      var $bits_8 = $bits_7;
      __label__ = 71;
      break;
     } else if (__label__ == 124) {
      var $bits_17;
      var $hold_17;
      var $have_21;
      var $next_21;
      var $116 = HEAP32[$24 >> 2];
      var $tobool616 = ($116 | 0) == 0;
      if ($tobool616) {
       var $next_22 = $next_21;
       var $have_22 = $have_21;
       var $hold_18 = $hold_17;
       var $bits_18 = $bits_17;
       while (1) {
        var $bits_18;
        var $hold_18;
        var $have_22;
        var $next_22;
        var $cmp629 = $bits_18 >>> 0 < 3;
        if (!$cmp629) {
         break;
        }
        var $cmp633 = ($have_22 | 0) == 0;
        if ($cmp633) {
         var $ret_8 = $ret_0;
         var $next_58 = $next_22;
         var $have_58 = 0;
         var $hold_54 = $hold_18;
         var $bits_54 = $bits_18;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $dec637 = $have_22 - 1 | 0;
        var $incdec_ptr638 = $next_22 + 1 | 0;
        var $117 = HEAPU8[$next_22];
        var $conv639 = $117 & 255;
        var $shl640 = $conv639 << $bits_18;
        var $add641 = $shl640 + $hold_18 | 0;
        var $add642 = $bits_18 + 8 | 0;
        var $next_22 = $incdec_ptr638;
        var $have_22 = $dec637;
        var $hold_18 = $add641;
        var $bits_18 = $add642;
       }
       var $and648 = $hold_18 & 1;
       HEAP32[$24 >> 2] = $and648;
       var $shr651 = $hold_18 >>> 1;
       var $and655 = $shr651 & 3;
       do {
        if (($and655 | 0) == 0) {
         HEAP32[$mode >> 2] = 13;
        } else if (($and655 | 0) == 1) {
         _fixedtables($4);
         HEAP32[$mode >> 2] = 19;
         if (!$cmp660) {
          break;
         }
         var $shr664 = $hold_18 >>> 3;
         var $sub665 = $bits_18 - 3 | 0;
         var $ret_8 = $ret_0;
         var $next_58 = $next_22;
         var $have_58 = $have_22;
         var $hold_54 = $shr664;
         var $bits_54 = $sub665;
         var $out_4 = $out_0;
         break $_$12;
        } else if (($and655 | 0) == 2) {
         HEAP32[$mode >> 2] = 16;
        } else if (($and655 | 0) == 3) {
         HEAP32[$msg >> 2] = STRING_TABLE.__str6103 | 0;
         HEAP32[$mode >> 2] = 29;
        }
       } while (0);
       var $shr675 = $hold_18 >>> 3;
       var $sub676 = $bits_18 - 3 | 0;
       var $ret_0_be = $ret_0;
       var $next_0_be = $next_22;
       var $put_0_be = $put_0;
       var $have_0_be = $have_22;
       var $left_0_be = $left_0;
       var $hold_0_be = $shr675;
       var $bits_0_be = $sub676;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      var $and619 = $bits_17 & 7;
      var $shr620 = $hold_17 >>> ($and619 >>> 0);
      var $sub622 = $bits_17 - $and619 | 0;
      HEAP32[$mode >> 2] = 26;
      var $ret_0_be = $ret_0;
      var $next_0_be = $next_21;
      var $put_0_be = $put_0;
      var $have_0_be = $have_21;
      var $left_0_be = $left_0;
      var $hold_0_be = $shr620;
      var $bits_0_be = $sub622;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (__label__ == 205) {
      var $bits_33;
      var $hold_33;
      var $have_37;
      var $next_37;
      var $ret_2;
      HEAP32[$mode >> 2] = 20;
      var $ret_3 = $ret_2;
      var $next_38 = $next_37;
      var $have_38 = $have_37;
      var $hold_34 = $hold_33;
      var $bits_34 = $bits_33;
      __label__ = 206;
      break;
     }
    } while (0);
    do {
     if (__label__ == 71) {
      var $bits_8;
      var $hold_8;
      var $have_8;
      var $next_8;
      var $77 = HEAPU32[$17 >> 2];
      var $and319 = $77 & 1024;
      var $tobool320 = ($and319 | 0) == 0;
      if ($tobool320) {
       var $next_10 = $next_8;
       var $have_10 = $have_8;
       var $88 = $77;
      } else {
       var $78 = HEAPU32[$21 >> 2];
       var $cmp323 = $78 >>> 0 > $have_8 >>> 0;
       var $copy_0 = $cmp323 ? $have_8 : $78;
       var $tobool327 = ($copy_0 | 0) == 0;
       if ($tobool327) {
        var $next_9 = $next_8;
        var $have_9 = $have_8;
        var $87 = $78;
        var $86 = $77;
       } else {
        var $79 = HEAPU32[$18 >> 2];
        var $cmp330 = ($79 | 0) == 0;
        do {
         if ($cmp330) {
          var $83 = $77;
         } else {
          var $extra334 = $79 + 16 | 0;
          var $80 = HEAP32[$extra334 >> 2];
          var $cmp335 = ($80 | 0) == 0;
          if ($cmp335) {
           var $83 = $77;
           break;
          }
          var $extra_len339 = $79 + 20 | 0;
          var $81 = HEAP32[$extra_len339 >> 2];
          var $sub341 = $81 - $78 | 0;
          var $add_ptr = $80 + $sub341 | 0;
          var $add344 = $sub341 + $copy_0 | 0;
          var $extra_max = $79 + 24 | 0;
          var $82 = HEAPU32[$extra_max >> 2];
          var $cmp346 = $add344 >>> 0 > $82 >>> 0;
          var $sub350 = $82 - $sub341 | 0;
          var $cond351 = $cmp346 ? $sub350 : $copy_0;
          _memcpy($add_ptr, $next_8, $cond351, 1);
          var $_pre886 = HEAP32[$17 >> 2];
          var $83 = $_pre886;
         }
        } while (0);
        var $83;
        var $and354 = $83 & 512;
        var $tobool355 = ($and354 | 0) == 0;
        if (!$tobool355) {
         var $84 = HEAP32[$16 >> 2];
         var $call358 = _crc32($84, $next_8, $copy_0);
         HEAP32[$16 >> 2] = $call358;
        }
        var $sub361 = $have_8 - $copy_0 | 0;
        var $add_ptr362 = $next_8 + $copy_0 | 0;
        var $85 = HEAP32[$21 >> 2];
        var $sub364 = $85 - $copy_0 | 0;
        HEAP32[$21 >> 2] = $sub364;
        var $next_9 = $add_ptr362;
        var $have_9 = $sub361;
        var $87 = $sub364;
        var $86 = $83;
       }
       var $86;
       var $87;
       var $have_9;
       var $next_9;
       var $tobool367 = ($87 | 0) == 0;
       if (!$tobool367) {
        var $ret_8 = $ret_0;
        var $next_58 = $next_9;
        var $have_58 = $have_9;
        var $hold_54 = $hold_8;
        var $bits_54 = $bits_8;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $next_10 = $next_9;
       var $have_10 = $have_9;
       var $88 = $86;
      }
      var $88;
      var $have_10;
      var $next_10;
      HEAP32[$21 >> 2] = 0;
      HEAP32[$mode >> 2] = 6;
      var $next_11 = $next_10;
      var $have_11 = $have_10;
      var $hold_9 = $hold_8;
      var $bits_9 = $bits_8;
      var $89 = $88;
      __label__ = 81;
      break;
     } else if (__label__ == 206) {
      var $bits_34;
      var $hold_34;
      var $have_38;
      var $next_38;
      var $ret_3;
      var $cmp1179 = $have_38 >>> 0 > 5;
      var $cmp1182 = $left_0 >>> 0 > 257;
      var $or_cond33 = $cmp1179 & $cmp1182;
      if ($or_cond33) {
       HEAP32[$next_out >> 2] = $put_0;
       HEAP32[$avail_out >> 2] = $left_0;
       HEAP32[$next_in >> 2] = $next_38;
       HEAP32[$avail_in15 >> 2] = $have_38;
       HEAP32[$11 >> 2] = $hold_34;
       HEAP32[$13 >> 2] = $bits_34;
       _inflate_fast($strm, $out_0);
       var $144 = HEAP32[$next_out >> 2];
       var $145 = HEAP32[$avail_out >> 2];
       var $146 = HEAP32[$next_in >> 2];
       var $147 = HEAP32[$avail_in15 >> 2];
       var $148 = HEAP32[$11 >> 2];
       var $149 = HEAP32[$13 >> 2];
       var $150 = HEAP32[$mode >> 2];
       var $cmp1204 = ($150 | 0) == 11;
       if (!$cmp1204) {
        var $ret_0_be = $ret_3;
        var $next_0_be = $146;
        var $put_0_be = $144;
        var $have_0_be = $147;
        var $left_0_be = $145;
        var $hold_0_be = $148;
        var $bits_0_be = $149;
        var $out_0_be = $out_0;
        __label__ = 268;
        break;
       }
       HEAP32[$25 >> 2] = -1;
       var $ret_0_be = $ret_3;
       var $next_0_be = $146;
       var $put_0_be = $144;
       var $have_0_be = $147;
       var $left_0_be = $145;
       var $hold_0_be = $148;
       var $bits_0_be = $149;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      HEAP32[$25 >> 2] = 0;
      var $151 = HEAP32[$26 >> 2];
      var $shl1212 = 1 << $151;
      var $sub1213 = $shl1212 - 1 | 0;
      var $152 = HEAPU32[$27 >> 2];
      var $next_39 = $next_38;
      var $have_39 = $have_38;
      var $hold_35 = $hold_34;
      var $bits_35 = $bits_34;
      while (1) {
       var $bits_35;
       var $hold_35;
       var $have_39;
       var $next_39;
       var $and1214 = $sub1213 & $hold_35;
       var $arrayidx1216_1 = $152 + ($and1214 << 2) + 1 | 0;
       var $tmp22 = HEAPU8[$arrayidx1216_1];
       var $conv1218 = $tmp22 & 255;
       var $cmp1219 = $conv1218 >>> 0 > $bits_35 >>> 0;
       if (!$cmp1219) {
        break;
       }
       var $cmp1224 = ($have_39 | 0) == 0;
       if ($cmp1224) {
        var $ret_8 = $ret_3;
        var $next_58 = $next_39;
        var $have_58 = 0;
        var $hold_54 = $hold_35;
        var $bits_54 = $bits_35;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec1228 = $have_39 - 1 | 0;
       var $incdec_ptr1229 = $next_39 + 1 | 0;
       var $153 = HEAPU8[$next_39];
       var $conv1230 = $153 & 255;
       var $shl1231 = $conv1230 << $bits_35;
       var $add1232 = $shl1231 + $hold_35 | 0;
       var $add1233 = $bits_35 + 8 | 0;
       var $next_39 = $incdec_ptr1229;
       var $have_39 = $dec1228;
       var $hold_35 = $add1232;
       var $bits_35 = $add1233;
      }
      var $arrayidx1216_0 = $152 + ($and1214 << 2) | 0;
      var $tmp21 = HEAPU8[$arrayidx1216_0];
      var $arrayidx1216_2 = $152 + ($and1214 << 2) + 2 | 0;
      var $tmp23 = HEAPU16[$arrayidx1216_2 >> 1];
      var $conv1237 = $tmp21 & 255;
      var $tobool1238 = $tmp21 << 24 >> 24 == 0;
      do {
       if ($tobool1238) {
        var $next_41 = $next_39;
        var $have_41 = $have_39;
        var $hold_37 = $hold_35;
        var $bits_37 = $bits_35;
        var $here_09_0 = 0;
        var $here_110_0 = $tmp22;
        var $here_211_0 = $tmp23;
        var $155 = 0;
       } else {
        var $and1242 = $conv1237 & 240;
        var $cmp1243 = ($and1242 | 0) == 0;
        if (!$cmp1243) {
         var $next_41 = $next_39;
         var $have_41 = $have_39;
         var $hold_37 = $hold_35;
         var $bits_37 = $bits_35;
         var $here_09_0 = $tmp21;
         var $here_110_0 = $tmp22;
         var $here_211_0 = $tmp23;
         var $155 = 0;
         break;
        }
        var $conv1248 = $tmp23 & 65535;
        var $add1253 = $conv1218 + $conv1237 | 0;
        var $shl1254 = 1 << $add1253;
        var $sub1255 = $shl1254 - 1 | 0;
        var $next_40 = $next_39;
        var $have_40 = $have_39;
        var $hold_36 = $hold_35;
        var $bits_36 = $bits_35;
        while (1) {
         var $bits_36;
         var $hold_36;
         var $have_40;
         var $next_40;
         var $and1256 = $hold_36 & $sub1255;
         var $shr1259 = $and1256 >>> ($conv1218 >>> 0);
         var $add1260 = $shr1259 + $conv1248 | 0;
         var $arrayidx1262_1 = $152 + ($add1260 << 2) + 1 | 0;
         var $tmp19 = HEAPU8[$arrayidx1262_1];
         var $conv1266 = $tmp19 & 255;
         var $add1267 = $conv1266 + $conv1218 | 0;
         var $cmp1268 = $add1267 >>> 0 > $bits_36 >>> 0;
         if (!$cmp1268) {
          break;
         }
         var $cmp1273 = ($have_40 | 0) == 0;
         if ($cmp1273) {
          var $ret_8 = $ret_3;
          var $next_58 = $next_40;
          var $have_58 = 0;
          var $hold_54 = $hold_36;
          var $bits_54 = $bits_36;
          var $out_4 = $out_0;
          break $_$12;
         }
         var $dec1277 = $have_40 - 1 | 0;
         var $incdec_ptr1278 = $next_40 + 1 | 0;
         var $154 = HEAPU8[$next_40];
         var $conv1279 = $154 & 255;
         var $shl1280 = $conv1279 << $bits_36;
         var $add1281 = $shl1280 + $hold_36 | 0;
         var $add1282 = $bits_36 + 8 | 0;
         var $next_40 = $incdec_ptr1278;
         var $have_40 = $dec1277;
         var $hold_36 = $add1281;
         var $bits_36 = $add1282;
        }
        var $arrayidx1262_2 = $152 + ($add1260 << 2) + 2 | 0;
        var $arrayidx1262_0 = $152 + ($add1260 << 2) | 0;
        var $tmp20 = HEAP16[$arrayidx1262_2 >> 1];
        var $tmp18 = HEAP8[$arrayidx1262_0];
        var $shr1289 = $hold_36 >>> ($conv1218 >>> 0);
        var $sub1292 = $bits_36 - $conv1218 | 0;
        HEAP32[$25 >> 2] = $conv1218;
        var $next_41 = $next_40;
        var $have_41 = $have_40;
        var $hold_37 = $shr1289;
        var $bits_37 = $sub1292;
        var $here_09_0 = $tmp18;
        var $here_110_0 = $tmp19;
        var $here_211_0 = $tmp20;
        var $155 = $conv1218;
       }
      } while (0);
      var $155;
      var $here_211_0;
      var $here_110_0;
      var $here_09_0;
      var $bits_37;
      var $hold_37;
      var $have_41;
      var $next_41;
      var $conv1302 = $here_110_0 & 255;
      var $shr1303 = $hold_37 >>> ($conv1302 >>> 0);
      var $sub1306 = $bits_37 - $conv1302 | 0;
      var $add1312 = $155 + $conv1302 | 0;
      HEAP32[$25 >> 2] = $add1312;
      var $conv1314 = $here_211_0 & 65535;
      HEAP32[$21 >> 2] = $conv1314;
      var $conv1317 = $here_09_0 & 255;
      var $cmp1318 = $here_09_0 << 24 >> 24 == 0;
      if ($cmp1318) {
       HEAP32[$mode >> 2] = 25;
       var $ret_0_be = $ret_3;
       var $next_0_be = $next_41;
       var $put_0_be = $put_0;
       var $have_0_be = $have_41;
       var $left_0_be = $left_0;
       var $hold_0_be = $shr1303;
       var $bits_0_be = $sub1306;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      var $and1325 = $conv1317 & 32;
      var $tobool1326 = ($and1325 | 0) == 0;
      if (!$tobool1326) {
       HEAP32[$25 >> 2] = -1;
       HEAP32[$mode >> 2] = 11;
       var $ret_0_be = $ret_3;
       var $next_0_be = $next_41;
       var $put_0_be = $put_0;
       var $have_0_be = $have_41;
       var $left_0_be = $left_0;
       var $hold_0_be = $shr1303;
       var $bits_0_be = $sub1306;
       var $out_0_be = $out_0;
       __label__ = 268;
       break;
      }
      var $and1333 = $conv1317 & 64;
      var $tobool1334 = ($and1333 | 0) == 0;
      if ($tobool1334) {
       var $and1341 = $conv1317 & 15;
       HEAP32[$28 >> 2] = $and1341;
       HEAP32[$mode >> 2] = 21;
       var $ret_4 = $ret_3;
       var $next_42 = $next_41;
       var $have_42 = $have_41;
       var $hold_38 = $shr1303;
       var $bits_38 = $sub1306;
       var $156 = $and1341;
       __label__ = 227;
       break;
      }
      HEAP32[$msg >> 2] = STRING_TABLE.__str2171 | 0;
      HEAP32[$mode >> 2] = 29;
      var $ret_0_be = $ret_3;
      var $next_0_be = $next_41;
      var $put_0_be = $put_0;
      var $have_0_be = $have_41;
      var $left_0_be = $left_0;
      var $hold_0_be = $shr1303;
      var $bits_0_be = $sub1306;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     }
    } while (0);
    do {
     if (__label__ == 81) {
      var $89;
      var $bits_9;
      var $hold_9;
      var $have_11;
      var $next_11;
      var $and375 = $89 & 2048;
      var $tobool376 = ($and375 | 0) == 0;
      do {
       if ($tobool376) {
        var $98 = HEAP32[$18 >> 2];
        var $cmp424 = ($98 | 0) == 0;
        if ($cmp424) {
         var $next_12 = $next_11;
         var $have_12 = $have_11;
         break;
        }
        var $name428 = $98 + 28 | 0;
        HEAP32[$name428 >> 2] = 0;
        var $next_12 = $next_11;
        var $have_12 = $have_11;
       } else {
        var $cmp378 = ($have_11 | 0) == 0;
        if ($cmp378) {
         var $ret_8 = $ret_0;
         var $next_58 = $next_11;
         var $have_58 = 0;
         var $hold_54 = $hold_9;
         var $bits_54 = $bits_9;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $copy_1 = 0;
        while (1) {
         var $copy_1;
         var $inc = $copy_1 + 1 | 0;
         var $arrayidx383 = $next_11 + $copy_1 | 0;
         var $90 = HEAP8[$arrayidx383];
         var $91 = HEAP32[$18 >> 2];
         var $cmp386 = ($91 | 0) == 0;
         do {
          if (!$cmp386) {
           var $name = $91 + 28 | 0;
           var $92 = HEAP32[$name >> 2];
           var $cmp390 = ($92 | 0) == 0;
           if ($cmp390) {
            break;
           }
           var $93 = HEAPU32[$21 >> 2];
           var $name_max = $91 + 32 | 0;
           var $94 = HEAPU32[$name_max >> 2];
           var $cmp395 = $93 >>> 0 < $94 >>> 0;
           if (!$cmp395) {
            break;
           }
           var $inc400 = $93 + 1 | 0;
           HEAP32[$21 >> 2] = $inc400;
           var $95 = HEAP32[$name >> 2];
           var $arrayidx403 = $95 + $93 | 0;
           HEAP8[$arrayidx403] = $90;
          }
         } while (0);
         var $tobool405 = $90 << 24 >> 24 != 0;
         var $cmp406 = $inc >>> 0 < $have_11 >>> 0;
         var $or_cond31 = $tobool405 & $cmp406;
         if (!$or_cond31) {
          break;
         }
         var $copy_1 = $inc;
        }
        var $96 = HEAP32[$17 >> 2];
        var $and410 = $96 & 512;
        var $tobool411 = ($and410 | 0) == 0;
        if (!$tobool411) {
         var $97 = HEAP32[$16 >> 2];
         var $call414 = _crc32($97, $next_11, $inc);
         HEAP32[$16 >> 2] = $call414;
        }
        var $sub417 = $have_11 - $inc | 0;
        var $add_ptr418 = $next_11 + $inc | 0;
        if ($tobool405) {
         var $ret_8 = $ret_0;
         var $next_58 = $add_ptr418;
         var $have_58 = $sub417;
         var $hold_54 = $hold_9;
         var $bits_54 = $bits_9;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $next_12 = $add_ptr418;
        var $have_12 = $sub417;
       }
      } while (0);
      var $have_12;
      var $next_12;
      HEAP32[$21 >> 2] = 0;
      HEAP32[$mode >> 2] = 7;
      var $next_13 = $next_12;
      var $have_13 = $have_12;
      var $hold_10 = $hold_9;
      var $bits_10 = $bits_9;
      __label__ = 94;
      break;
     } else if (__label__ == 227) {
      var $156;
      var $bits_38;
      var $hold_38;
      var $have_42;
      var $next_42;
      var $ret_4;
      var $tobool1346 = ($156 | 0) == 0;
      if ($tobool1346) {
       var $_pre890 = HEAP32[$21 >> 2];
       var $next_44 = $next_42;
       var $have_44 = $have_42;
       var $hold_40 = $hold_38;
       var $bits_40 = $bits_38;
       var $160 = $_pre890;
      } else {
       var $next_43 = $next_42;
       var $have_43 = $have_42;
       var $hold_39 = $hold_38;
       var $bits_39 = $bits_38;
       while (1) {
        var $bits_39;
        var $hold_39;
        var $have_43;
        var $next_43;
        var $cmp1351 = $bits_39 >>> 0 < $156 >>> 0;
        if (!$cmp1351) {
         break;
        }
        var $cmp1355 = ($have_43 | 0) == 0;
        if ($cmp1355) {
         var $ret_8 = $ret_4;
         var $next_58 = $next_43;
         var $have_58 = 0;
         var $hold_54 = $hold_39;
         var $bits_54 = $bits_39;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $dec1359 = $have_43 - 1 | 0;
        var $incdec_ptr1360 = $next_43 + 1 | 0;
        var $157 = HEAPU8[$next_43];
        var $conv1361 = $157 & 255;
        var $shl1362 = $conv1361 << $bits_39;
        var $add1363 = $shl1362 + $hold_39 | 0;
        var $add1364 = $bits_39 + 8 | 0;
        var $next_43 = $incdec_ptr1360;
        var $have_43 = $dec1359;
        var $hold_39 = $add1363;
        var $bits_39 = $add1364;
       }
       var $shl1371 = 1 << $156;
       var $sub1372 = $shl1371 - 1 | 0;
       var $and1373 = $sub1372 & $hold_39;
       var $158 = HEAP32[$21 >> 2];
       var $add1375 = $158 + $and1373 | 0;
       HEAP32[$21 >> 2] = $add1375;
       var $shr1378 = $hold_39 >>> ($156 >>> 0);
       var $sub1380 = $bits_39 - $156 | 0;
       var $159 = HEAP32[$25 >> 2];
       var $add1385 = $159 + $156 | 0;
       HEAP32[$25 >> 2] = $add1385;
       var $next_44 = $next_43;
       var $have_44 = $have_43;
       var $hold_40 = $shr1378;
       var $bits_40 = $sub1380;
       var $160 = $add1375;
      }
      var $160;
      var $bits_40;
      var $hold_40;
      var $have_44;
      var $next_44;
      HEAP32[$29 >> 2] = $160;
      HEAP32[$mode >> 2] = 22;
      var $ret_5_ph = $ret_4;
      var $next_45_ph = $next_44;
      var $have_45_ph = $have_44;
      var $hold_41_ph = $hold_40;
      var $bits_41_ph = $bits_40;
      __label__ = 234;
      break;
     }
    } while (0);
    do {
     if (__label__ == 94) {
      var $bits_10;
      var $hold_10;
      var $have_13;
      var $next_13;
      var $99 = HEAP32[$17 >> 2];
      var $and435 = $99 & 4096;
      var $tobool436 = ($and435 | 0) == 0;
      do {
       if ($tobool436) {
        var $108 = HEAP32[$18 >> 2];
        var $cmp488 = ($108 | 0) == 0;
        if ($cmp488) {
         var $next_14 = $next_13;
         var $have_14 = $have_13;
         break;
        }
        var $comment492 = $108 + 36 | 0;
        HEAP32[$comment492 >> 2] = 0;
        var $next_14 = $next_13;
        var $have_14 = $have_13;
       } else {
        var $cmp438 = ($have_13 | 0) == 0;
        if ($cmp438) {
         var $ret_8 = $ret_0;
         var $next_58 = $next_13;
         var $have_58 = 0;
         var $hold_54 = $hold_10;
         var $bits_54 = $bits_10;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $copy_2 = 0;
        while (1) {
         var $copy_2;
         var $inc443 = $copy_2 + 1 | 0;
         var $arrayidx444 = $next_13 + $copy_2 | 0;
         var $100 = HEAP8[$arrayidx444];
         var $101 = HEAP32[$18 >> 2];
         var $cmp447 = ($101 | 0) == 0;
         do {
          if (!$cmp447) {
           var $comment = $101 + 36 | 0;
           var $102 = HEAP32[$comment >> 2];
           var $cmp451 = ($102 | 0) == 0;
           if ($cmp451) {
            break;
           }
           var $103 = HEAPU32[$21 >> 2];
           var $comm_max = $101 + 40 | 0;
           var $104 = HEAPU32[$comm_max >> 2];
           var $cmp456 = $103 >>> 0 < $104 >>> 0;
           if (!$cmp456) {
            break;
           }
           var $inc461 = $103 + 1 | 0;
           HEAP32[$21 >> 2] = $inc461;
           var $105 = HEAP32[$comment >> 2];
           var $arrayidx464 = $105 + $103 | 0;
           HEAP8[$arrayidx464] = $100;
          }
         } while (0);
         var $tobool467 = $100 << 24 >> 24 != 0;
         var $cmp469 = $inc443 >>> 0 < $have_13 >>> 0;
         var $or_cond32 = $tobool467 & $cmp469;
         if (!$or_cond32) {
          break;
         }
         var $copy_2 = $inc443;
        }
        var $106 = HEAP32[$17 >> 2];
        var $and474 = $106 & 512;
        var $tobool475 = ($and474 | 0) == 0;
        if (!$tobool475) {
         var $107 = HEAP32[$16 >> 2];
         var $call478 = _crc32($107, $next_13, $inc443);
         HEAP32[$16 >> 2] = $call478;
        }
        var $sub481 = $have_13 - $inc443 | 0;
        var $add_ptr482 = $next_13 + $inc443 | 0;
        if ($tobool467) {
         var $ret_8 = $ret_0;
         var $next_58 = $add_ptr482;
         var $have_58 = $sub481;
         var $hold_54 = $hold_10;
         var $bits_54 = $bits_10;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $next_14 = $add_ptr482;
        var $have_14 = $sub481;
       }
      } while (0);
      var $have_14;
      var $next_14;
      HEAP32[$mode >> 2] = 8;
      var $next_15 = $next_14;
      var $have_15 = $have_14;
      var $hold_11 = $hold_10;
      var $bits_11 = $bits_10;
      __label__ = 107;
      break;
     } else if (__label__ == 234) {
      var $bits_41_ph;
      var $hold_41_ph;
      var $have_45_ph;
      var $next_45_ph;
      var $ret_5_ph;
      var $161 = HEAP32[$47 >> 2];
      var $shl1392 = 1 << $161;
      var $sub1393 = $shl1392 - 1 | 0;
      var $162 = HEAPU32[$48 >> 2];
      var $next_45 = $next_45_ph;
      var $have_45 = $have_45_ph;
      var $hold_41 = $hold_41_ph;
      var $bits_41 = $bits_41_ph;
      while (1) {
       var $bits_41;
       var $hold_41;
       var $have_45;
       var $next_45;
       var $and1394 = $sub1393 & $hold_41;
       var $arrayidx1396_1 = $162 + ($and1394 << 2) + 1 | 0;
       var $tmp16 = HEAPU8[$arrayidx1396_1];
       var $conv1398 = $tmp16 & 255;
       var $cmp1399 = $conv1398 >>> 0 > $bits_41 >>> 0;
       if (!$cmp1399) {
        break;
       }
       var $cmp1404 = ($have_45 | 0) == 0;
       if ($cmp1404) {
        var $ret_8 = $ret_5_ph;
        var $next_58 = $next_45;
        var $have_58 = 0;
        var $hold_54 = $hold_41;
        var $bits_54 = $bits_41;
        var $out_4 = $out_0;
        break $_$12;
       }
       var $dec1408 = $have_45 - 1 | 0;
       var $incdec_ptr1409 = $next_45 + 1 | 0;
       var $163 = HEAPU8[$next_45];
       var $conv1410 = $163 & 255;
       var $shl1411 = $conv1410 << $bits_41;
       var $add1412 = $shl1411 + $hold_41 | 0;
       var $add1413 = $bits_41 + 8 | 0;
       var $next_45 = $incdec_ptr1409;
       var $have_45 = $dec1408;
       var $hold_41 = $add1412;
       var $bits_41 = $add1413;
      }
      var $arrayidx1396_0 = $162 + ($and1394 << 2) | 0;
      var $tmp15 = HEAPU8[$arrayidx1396_0];
      var $arrayidx1396_2 = $162 + ($and1394 << 2) + 2 | 0;
      var $tmp17 = HEAPU16[$arrayidx1396_2 >> 1];
      var $conv1418 = $tmp15 & 255;
      var $and1419 = $conv1418 & 240;
      var $cmp1420 = ($and1419 | 0) == 0;
      if ($cmp1420) {
       var $conv1425 = $tmp17 & 65535;
       var $add1430 = $conv1398 + $conv1418 | 0;
       var $shl1431 = 1 << $add1430;
       var $sub1432 = $shl1431 - 1 | 0;
       var $next_46 = $next_45;
       var $have_46 = $have_45;
       var $hold_42 = $hold_41;
       var $bits_42 = $bits_41;
       while (1) {
        var $bits_42;
        var $hold_42;
        var $have_46;
        var $next_46;
        var $and1433 = $hold_42 & $sub1432;
        var $shr1436 = $and1433 >>> ($conv1398 >>> 0);
        var $add1437 = $shr1436 + $conv1425 | 0;
        var $arrayidx1439_1 = $162 + ($add1437 << 2) + 1 | 0;
        var $tmp13 = HEAPU8[$arrayidx1439_1];
        var $conv1443 = $tmp13 & 255;
        var $add1444 = $conv1443 + $conv1398 | 0;
        var $cmp1445 = $add1444 >>> 0 > $bits_42 >>> 0;
        if (!$cmp1445) {
         break;
        }
        var $cmp1450 = ($have_46 | 0) == 0;
        if ($cmp1450) {
         var $ret_8 = $ret_5_ph;
         var $next_58 = $next_46;
         var $have_58 = 0;
         var $hold_54 = $hold_42;
         var $bits_54 = $bits_42;
         var $out_4 = $out_0;
         break $_$12;
        }
        var $dec1454 = $have_46 - 1 | 0;
        var $incdec_ptr1455 = $next_46 + 1 | 0;
        var $164 = HEAPU8[$next_46];
        var $conv1456 = $164 & 255;
        var $shl1457 = $conv1456 << $bits_42;
        var $add1458 = $shl1457 + $hold_42 | 0;
        var $add1459 = $bits_42 + 8 | 0;
        var $next_46 = $incdec_ptr1455;
        var $have_46 = $dec1454;
        var $hold_42 = $add1458;
        var $bits_42 = $add1459;
       }
       var $arrayidx1439_2 = $162 + ($add1437 << 2) + 2 | 0;
       var $arrayidx1439_0 = $162 + ($add1437 << 2) | 0;
       var $tmp14 = HEAP16[$arrayidx1439_2 >> 1];
       var $tmp12 = HEAP8[$arrayidx1439_0];
       var $shr1466 = $hold_42 >>> ($conv1398 >>> 0);
       var $sub1469 = $bits_42 - $conv1398 | 0;
       var $165 = HEAP32[$25 >> 2];
       var $add1475 = $165 + $conv1398 | 0;
       HEAP32[$25 >> 2] = $add1475;
       var $next_47 = $next_46;
       var $have_47 = $have_46;
       var $hold_43 = $shr1466;
       var $bits_43 = $sub1469;
       var $here_09_1 = $tmp12;
       var $here_110_1 = $tmp13;
       var $here_211_1 = $tmp14;
       var $166 = $add1475;
      } else {
       var $_pre893 = HEAP32[$25 >> 2];
       var $next_47 = $next_45;
       var $have_47 = $have_45;
       var $hold_43 = $hold_41;
       var $bits_43 = $bits_41;
       var $here_09_1 = $tmp15;
       var $here_110_1 = $tmp16;
       var $here_211_1 = $tmp17;
       var $166 = $_pre893;
      }
      var $166;
      var $here_211_1;
      var $here_110_1;
      var $here_09_1;
      var $bits_43;
      var $hold_43;
      var $have_47;
      var $next_47;
      var $conv1479 = $here_110_1 & 255;
      var $shr1480 = $hold_43 >>> ($conv1479 >>> 0);
      var $sub1483 = $bits_43 - $conv1479 | 0;
      var $add1489 = $166 + $conv1479 | 0;
      HEAP32[$25 >> 2] = $add1489;
      var $conv1491 = $here_09_1 & 255;
      var $and1492 = $conv1491 & 64;
      var $tobool1493 = ($and1492 | 0) == 0;
      if ($tobool1493) {
       var $conv1499 = $here_211_1 & 65535;
       HEAP32[$30 >> 2] = $conv1499;
       var $and1502 = $conv1491 & 15;
       HEAP32[$28 >> 2] = $and1502;
       HEAP32[$mode >> 2] = 23;
       var $ret_6 = $ret_5_ph;
       var $next_48 = $next_47;
       var $have_48 = $have_47;
       var $hold_44 = $shr1480;
       var $bits_44 = $sub1483;
       var $167 = $and1502;
       __label__ = 248;
       break;
      }
      HEAP32[$msg >> 2] = STRING_TABLE.__str1170 | 0;
      HEAP32[$mode >> 2] = 29;
      var $ret_0_be = $ret_5_ph;
      var $next_0_be = $next_47;
      var $put_0_be = $put_0;
      var $have_0_be = $have_47;
      var $left_0_be = $left_0;
      var $hold_0_be = $shr1480;
      var $bits_0_be = $sub1483;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     }
    } while (0);
    $_$359 : do {
     if (__label__ == 107) {
      var $bits_11;
      var $hold_11;
      var $have_15;
      var $next_15;
      var $109 = HEAPU32[$17 >> 2];
      var $and498 = $109 & 512;
      var $tobool499 = ($and498 | 0) == 0;
      do {
       if (!$tobool499) {
        var $next_16 = $next_15;
        var $have_16 = $have_15;
        var $hold_12 = $hold_11;
        var $bits_12 = $bits_11;
        while (1) {
         var $bits_12;
         var $hold_12;
         var $have_16;
         var $next_16;
         var $cmp503 = $bits_12 >>> 0 < 16;
         if (!$cmp503) {
          break;
         }
         var $cmp507 = ($have_16 | 0) == 0;
         if ($cmp507) {
          var $ret_8 = $ret_0;
          var $next_58 = $next_16;
          var $have_58 = 0;
          var $hold_54 = $hold_12;
          var $bits_54 = $bits_12;
          var $out_4 = $out_0;
          break $_$12;
         }
         var $dec511 = $have_16 - 1 | 0;
         var $incdec_ptr512 = $next_16 + 1 | 0;
         var $110 = HEAPU8[$next_16];
         var $conv513 = $110 & 255;
         var $shl514 = $conv513 << $bits_12;
         var $add515 = $shl514 + $hold_12 | 0;
         var $add516 = $bits_12 + 8 | 0;
         var $next_16 = $incdec_ptr512;
         var $have_16 = $dec511;
         var $hold_12 = $add515;
         var $bits_12 = $add516;
        }
        var $111 = HEAP32[$16 >> 2];
        var $and523 = $111 & 65535;
        var $cmp524 = ($hold_12 | 0) == ($and523 | 0);
        if ($cmp524) {
         var $next_17 = $next_16;
         var $have_17 = $have_16;
         var $hold_13 = 0;
         var $bits_13 = 0;
         break;
        }
        HEAP32[$msg >> 2] = STRING_TABLE.__str5102 | 0;
        HEAP32[$mode >> 2] = 29;
        var $ret_0_be = $ret_0;
        var $next_0_be = $next_16;
        var $put_0_be = $put_0;
        var $have_0_be = $have_16;
        var $left_0_be = $left_0;
        var $hold_0_be = $hold_12;
        var $bits_0_be = $bits_12;
        var $out_0_be = $out_0;
        __label__ = 268;
        break $_$359;
       }
       var $next_17 = $next_15;
       var $have_17 = $have_15;
       var $hold_13 = $hold_11;
       var $bits_13 = $bits_11;
      } while (0);
      var $bits_13;
      var $hold_13;
      var $have_17;
      var $next_17;
      var $112 = HEAPU32[$18 >> 2];
      var $cmp535 = ($112 | 0) == 0;
      if (!$cmp535) {
       var $shr53930 = $109 >>> 9;
       var $and540 = $shr53930 & 1;
       var $hcrc = $112 + 44 | 0;
       HEAP32[$hcrc >> 2] = $and540;
       var $113 = HEAP32[$18 >> 2];
       var $done543 = $113 + 48 | 0;
       HEAP32[$done543 >> 2] = 1;
      }
      var $call545 = _crc32(0, 0, 0);
      HEAP32[$16 >> 2] = $call545;
      HEAP32[$adler >> 2] = $call545;
      HEAP32[$mode >> 2] = 11;
      var $ret_0_be = $ret_0;
      var $next_0_be = $next_17;
      var $put_0_be = $put_0;
      var $have_0_be = $have_17;
      var $left_0_be = $left_0;
      var $hold_0_be = $hold_13;
      var $bits_0_be = $bits_13;
      var $out_0_be = $out_0;
      __label__ = 268;
      break;
     } else if (__label__ == 248) {
      var $167;
      var $bits_44;
      var $hold_44;
      var $have_48;
      var $next_48;
      var $ret_6;
      var $tobool1507 = ($167 | 0) == 0;
      if ($tobool1507) {
       var $next_50 = $next_48;
       var $have_50 = $have_48;
       var $hold_46 = $hold_44;
       var $bits_46 = $bits_44;
      } else {
       var $next_49 = $next_48;
       var $have_49 = $have_48;
       var $hold_45 = $hold_44;
       var $bits_45 = $bits_44;
       while (1) {
        var $bits_45;
        var $hold_45;
        var $have_49;
        var $next_49;
        var $cmp1512 = $bits_45 >>> 0 < $167 >>> 0;
        if (!$cmp1512) {
         break;
        }
        var $cmp1516 = ($have_49 | 0) == 0;
        if ($cmp1516) {
         var $ret_8 = $ret_6;
         var $next_58 = $next_49;
         var $have_58 = 0;
         var $hold_54 = $hold_45;
         var $bits_54 = $bits_45;
         var $out_4 = $out_0;
         break $_$12;
        }
        // XXX first chunk with a bug-causing difference
        var $dec1520 = $have_49 - 1 | 0;
        var $incdec_ptr1521 = $next_49 + 1 | 0;
        var $168 = HEAPU8[$next_49];
        var $conv1522 = $168 & 255;
        var $shl1523 = $conv1522 << $bits_45;
        var $add1524 = $shl1523 + $hold_45 | 0;
        var $add1525 = $bits_45 + 8 | 0;
        var $next_49 = $incdec_ptr1521;
        var $have_49 = $dec1520;
        var $hold_45 = $add1524;
        var $bits_45 = $add1525;
       }
       var $shl1532 = 1 << $167;
       var $sub1533 = $shl1532 - 1 | 0;
       var $and1534 = $sub1533 & $hold_45;
       var $169 = HEAP32[$30 >> 2];
       var $add1536 = $169 + $and1534 | 0;
       HEAP32[$30 >> 2] = $add1536;
       var $shr1539 = $hold_45 >>> ($167 >>> 0);
       var $sub1541 = $bits_45 - $167 | 0;
       var $170 = HEAP32[$25 >> 2];
       var $add1546 = $170 + $167 | 0;
       HEAP32[$25 >> 2] = $add1546;
       var $next_50 = $next_49;
       var $have_50 = $have_49;
       var $hold_46 = $shr1539;
       var $bits_46 = $sub1541;
      }
      var $bits_46;
      var $hold_46;
      var $have_50;
      var $next_50;
      HEAP32[$mode >> 2] = 24;
      var $ret_7 = $ret_6;
      var $next_51 = $next_50;
      var $have_51 = $have_50;
      var $hold_47 = $hold_46;
      var $bits_47 = $bits_46;
      __label__ = 254;
      break;
     }
    } while (0);
    $_$380 : do {
     if (__label__ == 254) {
      var $bits_47;
      var $hold_47;
      var $have_51;
      var $next_51;
      var $ret_7;
      var $cmp1550 = ($left_0 | 0) == 0;
      if ($cmp1550) {
       var $ret_8 = $ret_7;
       var $next_58 = $next_51;
       var $have_58 = $have_51;
       var $hold_54 = $hold_47;
       var $bits_54 = $bits_47;
       var $out_4 = $out_0;
       break $_$12;
      }
      var $sub1554 = $out_0 - $left_0 | 0;
      var $171 = HEAPU32[$30 >> 2];
      var $cmp1556 = $171 >>> 0 > $sub1554 >>> 0;
      do {
       if ($cmp1556) {
        var $sub1560 = $171 - $sub1554 | 0;
        var $172 = HEAPU32[$31 >> 2];
        var $cmp1561 = $sub1560 >>> 0 > $172 >>> 0;
        do {
         if ($cmp1561) {
          var $173 = HEAP32[$32 >> 2];
          var $tobool1564 = ($173 | 0) == 0;
          if ($tobool1564) {
           break;
          }
          HEAP32[$msg >> 2] = STRING_TABLE.__str169 | 0;
          HEAP32[$mode >> 2] = 29;
          var $ret_0_be = $ret_7;
          var $next_0_be = $next_51;
          var $put_0_be = $put_0;
          var $have_0_be = $have_51;
          var $left_0_be = $left_0;
          var $hold_0_be = $hold_47;
          var $bits_0_be = $bits_47;
          var $out_0_be = $out_0;
          break $_$380;
         }
        } while (0);
        var $174 = HEAPU32[$33 >> 2];
        var $cmp1570 = $sub1560 >>> 0 > $174 >>> 0;
        if ($cmp1570) {
         var $sub1574 = $sub1560 - $174 | 0;
         var $175 = HEAP32[$34 >> 2];
         var $176 = HEAP32[$35 >> 2];
         var $sub1575 = $176 - $sub1574 | 0;
         var $add_ptr1576 = $175 + $sub1575 | 0;
         var $from_0 = $add_ptr1576;
         var $copy_7 = $sub1574;
        } else {
         var $177 = HEAP32[$34 >> 2];
         var $sub1580 = $174 - $sub1560 | 0;
         var $add_ptr1581 = $177 + $sub1580 | 0;
         var $from_0 = $add_ptr1581;
         var $copy_7 = $sub1560;
        }
        var $copy_7;
        var $from_0;
        var $178 = HEAPU32[$21 >> 2];
        var $cmp1584 = $copy_7 >>> 0 > $178 >>> 0;
        if (!$cmp1584) {
         var $from_1 = $from_0;
         var $copy_8 = $copy_7;
         var $180 = $178;
         break;
        }
        var $from_1 = $from_0;
        var $copy_8 = $178;
        var $180 = $178;
       } else {
        var $idx_neg = -$171 | 0;
        var $add_ptr1591 = $put_0 + $idx_neg | 0;
        var $179 = HEAP32[$21 >> 2];
        var $from_1 = $add_ptr1591;
        var $copy_8 = $179;
        var $180 = $179;
       }
      } while (0);
      var $180;
      var $copy_8;
      var $from_1;
      var $cmp1594 = $copy_8 >>> 0 > $left_0 >>> 0;
      var $copy_9 = $cmp1594 ? $left_0 : $copy_8;
      var $sub1600 = $180 - $copy_9 | 0;
      HEAP32[$21 >> 2] = $sub1600;
      var $181 = $copy_8 ^ -1;
      var $182 = $left_0 ^ -1;
      var $183 = $181 >>> 0 > $182 >>> 0;
      var $umax = $183 ? $181 : $182;
      var $from_2 = $from_1;
      var $put_1 = $put_0;
      var $copy_10 = $copy_9;
      while (1) {
       var $copy_10;
       var $put_1;
       var $from_2;
       var $incdec_ptr1602 = $from_2 + 1 | 0;
       var $184 = HEAP8[$from_2];
       var $incdec_ptr1603 = $put_1 + 1 | 0;
       HEAP8[$put_1] = $184;
       var $dec1605 = $copy_10 - 1 | 0;
       var $tobool1606 = ($dec1605 | 0) == 0;
       if ($tobool1606) {
        break;
       }
       var $from_2 = $incdec_ptr1602;
       var $put_1 = $incdec_ptr1603;
       var $copy_10 = $dec1605;
      }
      var $sub1598 = $left_0 - $copy_9 | 0;
      var $scevgep_sum = $umax ^ -1;
      var $scevgep632 = $put_0 + $scevgep_sum | 0;
      var $185 = HEAP32[$21 >> 2];
      var $cmp1609 = ($185 | 0) == 0;
      if (!$cmp1609) {
       var $ret_0_be = $ret_7;
       var $next_0_be = $next_51;
       var $put_0_be = $scevgep632;
       var $have_0_be = $have_51;
       var $left_0_be = $sub1598;
       var $hold_0_be = $hold_47;
       var $bits_0_be = $bits_47;
       var $out_0_be = $out_0;
       break;
      }
      HEAP32[$mode >> 2] = 20;
      var $ret_0_be = $ret_7;
      var $next_0_be = $next_51;
      var $put_0_be = $scevgep632;
      var $have_0_be = $have_51;
      var $left_0_be = $sub1598;
      var $hold_0_be = $hold_47;
      var $bits_0_be = $bits_47;
      var $out_0_be = $out_0;
     }
    } while (0);
    var $out_0_be;
    var $bits_0_be;
    var $hold_0_be;
    var $left_0_be;
    var $have_0_be;
    var $put_0_be;
    var $next_0_be;
    var $ret_0_be;
    var $_pre883 = HEAP32[$mode >> 2];
    var $ret_0 = $ret_0_be;
    var $next_0 = $next_0_be;
    var $put_0 = $put_0_be;
    var $have_0 = $have_0_be;
    var $left_0 = $left_0_be;
    var $hold_0 = $hold_0_be;
    var $bits_0 = $bits_0_be;
    var $out_0 = $out_0_be;
    var $49 = $_pre883;
   }
   var $out_4;
   var $bits_54;
   var $hold_54;
   var $have_58;
   var $next_58;
   var $ret_8;
   HEAP32[$next_out >> 2] = $put_0;
   HEAP32[$avail_out >> 2] = $left_0;
   HEAP32[$next_in >> 2] = $next_58;
   HEAP32[$avail_in15 >> 2] = $have_58;
   HEAP32[$11 >> 2] = $hold_54;
   HEAP32[$13 >> 2] = $bits_54;
   var $199 = HEAP32[$35 >> 2];
   var $tobool1755 = ($199 | 0) == 0;
   do {
    if ($tobool1755) {
     var $200 = HEAPU32[$mode >> 2];
     var $cmp1758 = $200 >>> 0 < 26;
     if (!$cmp1758) {
      __label__ = 300;
      break;
     }
     var $201 = HEAP32[$avail_out >> 2];
     var $cmp1762 = ($out_4 | 0) == ($201 | 0);
     if ($cmp1762) {
      __label__ = 300;
      break;
     }
     __label__ = 298;
     break;
    } else {
     __label__ = 298;
    }
   } while (0);
   do {
    if (__label__ == 298) {
     var $call1765 = _updatewindow($strm, $out_4);
     var $tobool1766 = ($call1765 | 0) == 0;
     if ($tobool1766) {
      break;
     }
     HEAP32[$mode >> 2] = 30;
     var $retval_0 = -4;
     break $_$2;
    }
   } while (0);
   var $202 = HEAPU32[$avail_in15 >> 2];
   var $203 = HEAPU32[$avail_out >> 2];
   var $sub1774 = $out_4 - $203 | 0;
   var $total_in = $strm + 8 | 0;
   var $204 = HEAP32[$total_in >> 2];
   var $sub1772 = $10 - $202 | 0;
   var $add1775 = $sub1772 + $204 | 0;
   HEAP32[$total_in >> 2] = $add1775;
   var $205 = HEAP32[$total_out >> 2];
   var $add1777 = $205 + $sub1774 | 0;
   HEAP32[$total_out >> 2] = $add1777;
   var $206 = HEAP32[$36 >> 2];
   var $add1779 = $206 + $sub1774 | 0;
   HEAP32[$36 >> 2] = $add1779;
   var $207 = HEAP32[$15 >> 2];
   var $tobool1781 = ($207 | 0) == 0;
   var $tobool1783 = ($out_4 | 0) == ($203 | 0);
   var $or_cond34 = $tobool1781 | $tobool1783;
   if (!$or_cond34) {
    var $208 = HEAP32[$17 >> 2];
    var $tobool1786 = ($208 | 0) == 0;
    var $209 = HEAP32[$16 >> 2];
    var $210 = HEAP32[$next_out >> 2];
    var $idx_neg1790 = -$sub1774 | 0;
    var $add_ptr1791 = $210 + $idx_neg1790 | 0;
    if ($tobool1786) {
     var $call1798 = _adler32($209, $add_ptr1791, $sub1774);
     var $cond1800 = $call1798;
    } else {
     var $call1792 = _crc32($209, $add_ptr1791, $sub1774);
     var $cond1800 = $call1792;
    }
    var $cond1800;
    HEAP32[$16 >> 2] = $cond1800;
    HEAP32[$adler >> 2] = $cond1800;
   }
   var $211 = HEAP32[$13 >> 2];
   var $212 = HEAP32[$24 >> 2];
   var $tobool1806 = ($212 | 0) != 0;
   var $cond1807 = $tobool1806 ? 64 : 0;
   var $213 = HEAP32[$mode >> 2];
   var $cmp1810 = ($213 | 0) == 11;
   var $cond1812 = $cmp1810 ? 128 : 0;
   var $cmp1815 = ($213 | 0) == 19;
   if ($cmp1815) {
    var $214 = 256;
   } else {
    var $cmp1818 = ($213 | 0) == 14;
    var $phitmp = $cmp1818 ? 256 : 0;
    var $214 = $phitmp;
   }
   var $214;
   var $add1808 = $cond1807 + $211 | 0;
   var $add1813 = $add1808 + $cond1812 | 0;
   var $add1821 = $add1813 + $214 | 0;
   var $data_type = $strm + 44 | 0;
   HEAP32[$data_type >> 2] = $add1821;
   var $cmp1822 = ($10 | 0) == ($202 | 0);
   var $or_cond35 = $cmp1822 & $tobool1783;
   var $cmp1828 = ($flush | 0) == 4;
   var $or_cond36 = $or_cond35 | $cmp1828;
   var $cmp1831 = ($ret_8 | 0) == 0;
   var $or_cond37 = $or_cond36 & $cmp1831;
   var $ret_9 = $or_cond37 ? -5 : $ret_8;
   var $retval_0 = $ret_9;
  }
 } while (0);
 var $retval_0;
 STACKTOP = __stackBase__;
 return $retval_0;
 return null;
}

function _malloc($bytes) {
 var __label__;
 var $1 = $bytes >>> 0 < 245;
 $_$2 : do {
  if ($1) {
   var $3 = $bytes >>> 0 < 11;
   if ($3) {
    var $8 = 16;
   } else {
    var $5 = $bytes + 11 | 0;
    var $6 = $5 & -8;
    var $8 = $6;
   }
   var $8;
   var $9 = $8 >>> 3;
   var $10 = HEAPU32[(__gm_ | 0) >> 2];
   var $11 = $10 >>> ($9 >>> 0);
   var $12 = $11 & 3;
   var $13 = ($12 | 0) == 0;
   if (!$13) {
    var $15 = $11 & 1;
    var $16 = $15 ^ 1;
    var $17 = $16 + $9 | 0;
    var $18 = $17 << 1;
    var $19 = __gm_ + 40 + ($18 << 2) | 0;
    var $20 = $19;
    var $_sum10 = $18 + 2 | 0;
    var $21 = __gm_ + 40 + ($_sum10 << 2) | 0;
    var $22 = HEAPU32[$21 >> 2];
    var $23 = $22 + 8 | 0;
    var $24 = HEAPU32[$23 >> 2];
    var $25 = ($20 | 0) == ($24 | 0);
    if ($25) {
     var $27 = 1 << $17;
     var $28 = $27 ^ -1;
     var $29 = $10 & $28;
     HEAP32[(__gm_ | 0) >> 2] = $29;
    } else {
     var $31 = $24;
     var $32 = HEAPU32[(__gm_ + 16 | 0) >> 2];
     var $33 = $31 >>> 0 < $32 >>> 0;
     if ($33) {
      _abort();
     } else {
      HEAP32[$21 >> 2] = $24;
      var $35 = $24 + 12 | 0;
      HEAP32[$35 >> 2] = $20;
     }
    }
    var $38 = $17 << 3;
    var $39 = $38 | 3;
    var $40 = $22 + 4 | 0;
    HEAP32[$40 >> 2] = $39;
    var $41 = $22;
    var $_sum1112 = $38 | 4;
    var $42 = $41 + $_sum1112 | 0;
    var $43 = $42;
    var $44 = HEAP32[$43 >> 2];
    var $45 = $44 | 1;
    HEAP32[$43 >> 2] = $45;
    var $46 = $23;
    var $mem_0 = $46;
    __label__ = 331;
    break;
   }
   var $48 = HEAPU32[(__gm_ + 8 | 0) >> 2];
   var $49 = $8 >>> 0 > $48 >>> 0;
   if (!$49) {
    var $nb_0 = $8;
    __label__ = 155;
    break;
   }
   var $51 = ($11 | 0) == 0;
   if (!$51) {
    var $53 = $11 << $9;
    var $54 = 2 << $9;
    var $55 = -$54 | 0;
    var $56 = $54 | $55;
    var $57 = $53 & $56;
    var $58 = -$57 | 0;
    var $59 = $57 & $58;
    var $60 = $59 - 1 | 0;
    var $61 = $60 >>> 12;
    var $62 = $61 & 16;
    var $63 = $60 >>> ($62 >>> 0);
    var $64 = $63 >>> 5;
    var $65 = $64 & 8;
    var $66 = $63 >>> ($65 >>> 0);
    var $67 = $66 >>> 2;
    var $68 = $67 & 4;
    var $69 = $66 >>> ($68 >>> 0);
    var $70 = $69 >>> 1;
    var $71 = $70 & 2;
    var $72 = $69 >>> ($71 >>> 0);
    var $73 = $72 >>> 1;
    var $74 = $73 & 1;
    var $75 = $65 | $62;
    var $76 = $75 | $68;
    var $77 = $76 | $71;
    var $78 = $77 | $74;
    var $79 = $72 >>> ($74 >>> 0);
    var $80 = $78 + $79 | 0;
    var $81 = $80 << 1;
    var $82 = __gm_ + 40 + ($81 << 2) | 0;
    var $83 = $82;
    var $_sum4 = $81 + 2 | 0;
    var $84 = __gm_ + 40 + ($_sum4 << 2) | 0;
    var $85 = HEAPU32[$84 >> 2];
    var $86 = $85 + 8 | 0;
    var $87 = HEAPU32[$86 >> 2];
    var $88 = ($83 | 0) == ($87 | 0);
    if ($88) {
     var $90 = 1 << $80;
     var $91 = $90 ^ -1;
     var $92 = $10 & $91;
     HEAP32[(__gm_ | 0) >> 2] = $92;
    } else {
     var $94 = $87;
     var $95 = HEAPU32[(__gm_ + 16 | 0) >> 2];
     var $96 = $94 >>> 0 < $95 >>> 0;
     if ($96) {
      _abort();
     } else {
      HEAP32[$84 >> 2] = $87;
      var $98 = $87 + 12 | 0;
      HEAP32[$98 >> 2] = $83;
     }
    }
    var $101 = $80 << 3;
    var $102 = $101 - $8 | 0;
    var $103 = $8 | 3;
    var $104 = $85 + 4 | 0;
    HEAP32[$104 >> 2] = $103;
    var $105 = $85;
    var $106 = $105 + $8 | 0;
    var $107 = $106;
    var $108 = $102 | 1;
    var $_sum56 = $8 | 4;
    var $109 = $105 + $_sum56 | 0;
    var $110 = $109;
    HEAP32[$110 >> 2] = $108;
    var $111 = $105 + $101 | 0;
    var $112 = $111;
    HEAP32[$112 >> 2] = $102;
    var $113 = HEAPU32[(__gm_ + 8 | 0) >> 2];
    var $114 = ($113 | 0) == 0;
    if (!$114) {
     var $116 = HEAP32[(__gm_ + 20 | 0) >> 2];
     var $117 = $113 >>> 3;
     var $118 = $113 >>> 2;
     var $119 = $118 & 1073741822;
     var $120 = __gm_ + 40 + ($119 << 2) | 0;
     var $121 = $120;
     var $122 = HEAPU32[(__gm_ | 0) >> 2];
     var $123 = 1 << $117;
     var $124 = $122 & $123;
     var $125 = ($124 | 0) == 0;
     do {
      if ($125) {
       var $127 = $122 | $123;
       HEAP32[(__gm_ | 0) >> 2] = $127;
       var $_sum8_pre = $119 + 2 | 0;
       var $_pre = __gm_ + 40 + ($_sum8_pre << 2) | 0;
       var $F4_0 = $121;
       var $_pre_phi = $_pre;
      } else {
       var $_sum9 = $119 + 2 | 0;
       var $129 = __gm_ + 40 + ($_sum9 << 2) | 0;
       var $130 = HEAPU32[$129 >> 2];
       var $131 = $130;
       var $132 = HEAPU32[(__gm_ + 16 | 0) >> 2];
       var $133 = $131 >>> 0 < $132 >>> 0;
       if (!$133) {
        var $F4_0 = $130;
        var $_pre_phi = $129;
        break;
       }
       _abort();
      }
     } while (0);
     var $_pre_phi;
     var $F4_0;
     HEAP32[$_pre_phi >> 2] = $116;
     var $136 = $F4_0 + 12 | 0;
     HEAP32[$136 >> 2] = $116;
     var $137 = $116 + 8 | 0;
     HEAP32[$137 >> 2] = $F4_0;
     var $138 = $116 + 12 | 0;
     HEAP32[$138 >> 2] = $121;
    }
    HEAP32[(__gm_ + 8 | 0) >> 2] = $102;
    HEAP32[(__gm_ + 20 | 0) >> 2] = $107;
    var $140 = $86;
    var $mem_0 = $140;
    __label__ = 331;
    break;
   }
   var $142 = HEAP32[(__gm_ + 4 | 0) >> 2];
   var $143 = ($142 | 0) == 0;
   if ($143) {
    var $nb_0 = $8;
    __label__ = 155;
    break;
   }
   var $145 = -$142 | 0;
   var $146 = $142 & $145;
   var $147 = $146 - 1 | 0;
   var $148 = $147 >>> 12;
   var $149 = $148 & 16;
   var $150 = $147 >>> ($149 >>> 0);
   var $151 = $150 >>> 5;
   var $152 = $151 & 8;
   var $153 = $150 >>> ($152 >>> 0);
   var $154 = $153 >>> 2;
   var $155 = $154 & 4;
   var $156 = $153 >>> ($155 >>> 0);
   var $157 = $156 >>> 1;
   var $158 = $157 & 2;
   var $159 = $156 >>> ($158 >>> 0);
   var $160 = $159 >>> 1;
   var $161 = $160 & 1;
   var $162 = $152 | $149;
   var $163 = $162 | $155;
   var $164 = $163 | $158;
   var $165 = $164 | $161;
   var $166 = $159 >>> ($161 >>> 0);
   var $167 = $165 + $166 | 0;
   var $168 = __gm_ + 304 + ($167 << 2) | 0;
   var $169 = HEAPU32[$168 >> 2];
   var $170 = $169 + 4 | 0;
   var $171 = HEAP32[$170 >> 2];
   var $172 = $171 & -8;
   var $173 = $172 - $8 | 0;
   var $t_0_i = $169;
   var $v_0_i = $169;
   var $rsize_0_i = $173;
   while (1) {
    var $rsize_0_i;
    var $v_0_i;
    var $t_0_i;
    var $175 = $t_0_i + 16 | 0;
    var $176 = HEAP32[$175 >> 2];
    var $177 = ($176 | 0) == 0;
    if ($177) {
     var $179 = $t_0_i + 20 | 0;
     var $180 = HEAP32[$179 >> 2];
     var $181 = ($180 | 0) == 0;
     if ($181) {
      break;
     }
     var $182 = $180;
    } else {
     var $182 = $176;
    }
    var $182;
    var $183 = $182 + 4 | 0;
    var $184 = HEAP32[$183 >> 2];
    var $185 = $184 & -8;
    var $186 = $185 - $8 | 0;
    var $187 = $186 >>> 0 < $rsize_0_i >>> 0;
    var $_rsize_0_i = $187 ? $186 : $rsize_0_i;
    var $_v_0_i = $187 ? $182 : $v_0_i;
    var $t_0_i = $182;
    var $v_0_i = $_v_0_i;
    var $rsize_0_i = $_rsize_0_i;
   }
   var $189 = $v_0_i;
   var $190 = HEAPU32[(__gm_ + 16 | 0) >> 2];
   var $191 = $189 >>> 0 < $190 >>> 0;
   do {
    if (!$191) {
     var $193 = $189 + $8 | 0;
     var $194 = $193;
     var $195 = $189 >>> 0 < $193 >>> 0;
     if (!$195) {
      break;
     }
     var $197 = $v_0_i + 24 | 0;
     var $198 = HEAPU32[$197 >> 2];
     var $199 = $v_0_i + 12 | 0;
     var $200 = HEAPU32[$199 >> 2];
     var $201 = ($200 | 0) == ($v_0_i | 0);
     do {
      if ($201) {
       var $212 = $v_0_i + 20 | 0;
       var $213 = HEAP32[$212 >> 2];
       var $214 = ($213 | 0) == 0;
       if ($214) {
        var $216 = $v_0_i + 16 | 0;
        var $217 = HEAP32[$216 >> 2];
        var $218 = ($217 | 0) == 0;
        if ($218) {
         var $R_1_i = 0;
         break;
        }
        var $RP_0_i = $216;
        var $R_0_i = $217;
       } else {
        var $RP_0_i = $212;
        var $R_0_i = $213;
        __label__ = 39;
       }
       while (1) {
        var $R_0_i;
        var $RP_0_i;
        var $219 = $R_0_i + 20 | 0;
        var $220 = HEAP32[$219 >> 2];
        var $221 = ($220 | 0) == 0;
        if (!$221) {
         var $RP_0_i = $219;
         var $R_0_i = $220;
         continue;
        }
        var $223 = $R_0_i + 16 | 0;
        var $224 = HEAPU32[$223 >> 2];
        var $225 = ($224 | 0) == 0;
        if ($225) {
         break;
        }
        var $RP_0_i = $223;
        var $R_0_i = $224;
       }
       var $227 = $RP_0_i;
       var $228 = $227 >>> 0 < $190 >>> 0;
       if ($228) {
        _abort();
       } else {
        HEAP32[$RP_0_i >> 2] = 0;
        var $R_1_i = $R_0_i;
       }
      } else {
       var $203 = $v_0_i + 8 | 0;
       var $204 = HEAPU32[$203 >> 2];
       var $205 = $204;
       var $206 = $205 >>> 0 < $190 >>> 0;
       if ($206) {
        _abort();
       } else {
        var $208 = $204 + 12 | 0;
        HEAP32[$208 >> 2] = $200;
        var $209 = $200 + 8 | 0;
        HEAP32[$209 >> 2] = $204;
        var $R_1_i = $200;
       }
      }
     } while (0);
     var $R_1_i;
     var $232 = ($198 | 0) == 0;
     $_$62 : do {
      if (!$232) {
       var $234 = $v_0_i + 28 | 0;
       var $235 = HEAP32[$234 >> 2];
       var $236 = __gm_ + 304 + ($235 << 2) | 0;
       var $237 = HEAP32[$236 >> 2];
       var $238 = ($v_0_i | 0) == ($237 | 0);
       do {
        if ($238) {
         HEAP32[$236 >> 2] = $R_1_i;
         var $cond_i = ($R_1_i | 0) == 0;
         if (!$cond_i) {
          break;
         }
         var $240 = HEAP32[$234 >> 2];
         var $241 = 1 << $240;
         var $242 = $241 ^ -1;
         var $243 = HEAP32[(__gm_ + 4 | 0) >> 2];
         var $244 = $243 & $242;
         HEAP32[(__gm_ + 4 | 0) >> 2] = $244;
         break $_$62;
        }
        var $246 = $198;
        var $247 = HEAPU32[(__gm_ + 16 | 0) >> 2];
        var $248 = $246 >>> 0 < $247 >>> 0;
        if ($248) {
         _abort();
        } else {
         var $250 = $198 + 16 | 0;
         var $251 = HEAP32[$250 >> 2];
         var $252 = ($251 | 0) == ($v_0_i | 0);
         if ($252) {
          HEAP32[$250 >> 2] = $R_1_i;
         } else {
          var $255 = $198 + 20 | 0;
          HEAP32[$255 >> 2] = $R_1_i;
         }
         var $258 = ($R_1_i | 0) == 0;
         if ($258) {
          break $_$62;
         }
        }
       } while (0);
       var $260 = $R_1_i;
       var $261 = HEAPU32[(__gm_ + 16 | 0) >> 2];
       var $262 = $260 >>> 0 < $261 >>> 0;
       if ($262) {
        _abort();
       } else {
        var $264 = $R_1_i + 24 | 0;
        HEAP32[$264 >> 2] = $198;
        var $265 = $v_0_i + 16 | 0;
        var $266 = HEAPU32[$265 >> 2];
        var $267 = ($266 | 0) == 0;
        if (!$267) {
         var $269 = $266;
         var $270 = HEAPU32[(__gm_ + 16 | 0) >> 2];
         var $271 = $269 >>> 0 < $270 >>> 0;
         if ($271) {
          _abort();
         } else {
          var $273 = $R_1_i + 16 | 0;
          HEAP32[$273 >> 2] = $266;
          var $274 = $266 + 24 | 0;
          HEAP32[$274 >> 2] = $R_1_i;
         }
        }
        var $277 = $v_0_i + 20 | 0;
        var $278 = HEAPU32[$277 >> 2];
        var $279 = ($278 | 0) == 0;
        if ($279) {
         break;
        }
        var $281 = $278;
        var $282 = HEAPU32[(__gm_ + 16 | 0) >> 2];
        var $283 = $281 >>> 0 < $282 >>> 0;
        if ($283) {
         _abort();
        } else {
         var $285 = $R_1_i + 20 | 0;
         HEAP32[$285 >> 2] = $278;
         var $286 = $278 + 24 | 0;
         HEAP32[$286 >> 2] = $R_1_i;
        }
       }
      }
     } while (0);
     var $290 = $rsize_0_i >>> 0 < 16;
     if ($290) {
      var $292 = $rsize_0_i + $8 | 0;
      var $293 = $292 | 3;
      var $294 = $v_0_i + 4 | 0;
      HEAP32[$294 >> 2] = $293;
      var $_sum4_i = $292 + 4 | 0;
      var $295 = $189 + $_sum4_i | 0;
      var $296 = $295;
      var $297 = HEAP32[$296 >> 2];
      var $298 = $297 | 1;
      HEAP32[$296 >> 2] = $298;
     } else {
      var $300 = $8 | 3;
      var $301 = $v_0_i + 4 | 0;
      HEAP32[$301 >> 2] = $300;
      var $302 = $rsize_0_i | 1;
      var $_sum_i33 = $8 | 4;
      var $303 = $189 + $_sum_i33 | 0;
      var $304 = $303;
      HEAP32[$304 >> 2] = $302;
      var $_sum1_i = $rsize_0_i + $8 | 0;
      var $305 = $189 + $_sum1_i | 0;
      var $306 = $305;
      HEAP32[$306 >> 2] = $rsize_0_i;
      var $307 = HEAPU32[(__gm_ + 8 | 0) >> 2];
      var $308 = ($307 | 0) == 0;
      if (!$308) {
       var $310 = HEAPU32[(__gm_ + 20 | 0) >> 2];
       var $311 = $307 >>> 3;
       var $312 = $307 >>> 2;
       var $313 = $312 & 1073741822;
       var $314 = __gm_ + 40 + ($313 << 2) | 0;
       var $315 = $314;
       var $316 = HEAPU32[(__gm_ | 0) >> 2];
       var $317 = 1 << $311;
       var $318 = $316 & $317;
       var $319 = ($318 | 0) == 0;
       do {
        if ($319) {
         var $321 = $316 | $317;
         HEAP32[(__gm_ | 0) >> 2] = $321;
         var $_sum2_pre_i = $313 + 2 | 0;
         var $_pre_i = __gm_ + 40 + ($_sum2_pre_i << 2) | 0;
         var $F1_0_i = $315;
         var $_pre_phi_i = $_pre_i;
        } else {
         var $_sum3_i = $313 + 2 | 0;
         var $323 = __gm_ + 40 + ($_sum3_i << 2) | 0;
         var $324 = HEAPU32[$323 >> 2];
         var $325 = $324;
         var $326 = HEAPU32[(__gm_ + 16 | 0) >> 2];
         var $327 = $325 >>> 0 < $326 >>> 0;
         if (!$327) {
          var $F1_0_i = $324;
          var $_pre_phi_i = $323;
          break;
         }
         _abort();
        }
       } while (0);
       var $_pre_phi_i;
       var $F1_0_i;
       HEAP32[$_pre_phi_i >> 2] = $310;
       var $330 = $F1_0_i + 12 | 0;
       HEAP32[$330 >> 2] = $310;
       var $331 = $310 + 8 | 0;
       HEAP32[$331 >> 2] = $F1_0_i;
       var $332 = $310 + 12 | 0;
       HEAP32[$332 >> 2] = $315;
      }
      HEAP32[(__gm_ + 8 | 0) >> 2] = $rsize_0_i;
      HEAP32[(__gm_ + 20 | 0) >> 2] = $194;
     }
     var $335 = $v_0_i + 8 | 0;
     var $336 = $335;
     var $337 = ($335 | 0) == 0;
     if ($337) {
      var $nb_0 = $8;
      __label__ = 155;
      break $_$2;
     }
     var $mem_0 = $336;
     __label__ = 331;
     break $_$2;
    }
   } while (0);
   _abort();
  } else {
   var $339 = $bytes >>> 0 > 4294967231;
   if ($339) {
    var $nb_0 = -1;
    __label__ = 155;
    break;
   }
   var $341 = $bytes + 11 | 0;
   var $342 = $341 & -8;
   var $343 = HEAPU32[(__gm_ + 4 | 0) >> 2];
   var $344 = ($343 | 0) == 0;
   if ($344) {
    var $nb_0 = $342;
    __label__ = 155;
    break;
   }
   var $346 = -$342 | 0;
   var $347 = $341 >>> 8;
   var $348 = ($347 | 0) == 0;
   do {
    if ($348) {
     var $idx_0_i = 0;
    } else {
     var $350 = $342 >>> 0 > 16777215;
     if ($350) {
      var $idx_0_i = 31;
      break;
     }
     var $352 = $347 + 1048320 | 0;
     var $353 = $352 >>> 16;
     var $354 = $353 & 8;
     var $355 = $347 << $354;
     var $356 = $355 + 520192 | 0;
     var $357 = $356 >>> 16;
     var $358 = $357 & 4;
     var $359 = $355 << $358;
     var $360 = $359 + 245760 | 0;
     var $361 = $360 >>> 16;
     var $362 = $361 & 2;
     var $363 = $358 | $354;
     var $364 = $363 | $362;
     var $365 = 14 - $364 | 0;
     var $366 = $359 << $362;
     var $367 = $366 >>> 15;
     var $368 = $365 + $367 | 0;
     var $369 = $368 << 1;
     var $370 = $368 + 7 | 0;
     var $371 = $342 >>> ($370 >>> 0);
     var $372 = $371 & 1;
     var $373 = $372 | $369;
     var $idx_0_i = $373;
    }
   } while (0);
   var $idx_0_i;
   var $375 = __gm_ + 304 + ($idx_0_i << 2) | 0;
   var $376 = HEAPU32[$375 >> 2];
   var $377 = ($376 | 0) == 0;
   $_$110 : do {
    if ($377) {
     var $v_2_i = 0;
     var $rsize_2_i = $346;
     var $t_1_i = 0;
    } else {
     var $379 = ($idx_0_i | 0) == 31;
     if ($379) {
      var $384 = 0;
     } else {
      var $381 = $idx_0_i >>> 1;
      var $382 = 25 - $381 | 0;
      var $384 = $382;
     }
     var $384;
     var $385 = $342 << $384;
     var $v_0_i15 = 0;
     var $rsize_0_i14 = $346;
     var $t_0_i13 = $376;
     var $sizebits_0_i = $385;
     var $rst_0_i = 0;
     while (1) {
      var $rst_0_i;
      var $sizebits_0_i;
      var $t_0_i13;
      var $rsize_0_i14;
      var $v_0_i15;
      var $387 = $t_0_i13 + 4 | 0;
      var $388 = HEAP32[$387 >> 2];
      var $389 = $388 & -8;
      var $390 = $389 - $342 | 0;
      var $391 = $390 >>> 0 < $rsize_0_i14 >>> 0;
      if ($391) {
       var $393 = ($389 | 0) == ($342 | 0);
       if ($393) {
        var $v_2_i = $t_0_i13;
        var $rsize_2_i = $390;
        var $t_1_i = $t_0_i13;
        break $_$110;
       }
       var $v_1_i = $t_0_i13;
       var $rsize_1_i = $390;
      } else {
       var $v_1_i = $v_0_i15;
       var $rsize_1_i = $rsize_0_i14;
      }
      var $rsize_1_i;
      var $v_1_i;
      var $395 = $t_0_i13 + 20 | 0;
      var $396 = HEAPU32[$395 >> 2];
      var $397 = $sizebits_0_i >>> 31;
      var $398 = $t_0_i13 + 16 + ($397 << 2) | 0;
      var $399 = HEAPU32[$398 >> 2];
      var $400 = ($396 | 0) == 0;
      var $401 = ($396 | 0) == ($399 | 0);
      var $or_cond_i = $400 | $401;
      var $rst_1_i = $or_cond_i ? $rst_0_i : $396;
      var $402 = ($399 | 0) == 0;
      var $403 = $sizebits_0_i << 1;
      if ($402) {
       var $v_2_i = $v_1_i;
       var $rsize_2_i = $rsize_1_i;
       var $t_1_i = $rst_1_i;
       break $_$110;
      }
      var $v_0_i15 = $v_1_i;
      var $rsize_0_i14 = $rsize_1_i;
      var $t_0_i13 = $399;
      var $sizebits_0_i = $403;
      var $rst_0_i = $rst_1_i;
     }
    }
   } while (0);
   var $t_1_i;
   var $rsize_2_i;
   var $v_2_i;
   var $404 = ($t_1_i | 0) == 0;
   var $405 = ($v_2_i | 0) == 0;
   var $or_cond19_i = $404 & $405;
   if ($or_cond19_i) {
    var $407 = 2 << $idx_0_i;
    var $408 = -$407 | 0;
    var $409 = $407 | $408;
    var $410 = $343 & $409;
    var $411 = ($410 | 0) == 0;
    if ($411) {
     var $nb_0 = $342;
     __label__ = 155;
     break;
    }
    var $413 = -$410 | 0;
    var $414 = $410 & $413;
    var $415 = $414 - 1 | 0;
    var $416 = $415 >>> 12;
    var $417 = $416 & 16;
    var $418 = $415 >>> ($417 >>> 0);
    var $419 = $418 >>> 5;
    var $420 = $419 & 8;
    var $421 = $418 >>> ($420 >>> 0);
    var $422 = $421 >>> 2;
    var $423 = $422 & 4;
    var $424 = $421 >>> ($423 >>> 0);
    var $425 = $424 >>> 1;
    var $426 = $425 & 2;
    var $427 = $424 >>> ($426 >>> 0);
    var $428 = $427 >>> 1;
    var $429 = $428 & 1;
    var $430 = $420 | $417;
    var $431 = $430 | $423;
    var $432 = $431 | $426;
    var $433 = $432 | $429;
    var $434 = $427 >>> ($429 >>> 0);
    var $435 = $433 + $434 | 0;
    var $436 = __gm_ + 304 + ($435 << 2) | 0;
    var $437 = HEAP32[$436 >> 2];
    var $t_2_ph_i = $437;
   } else {
    var $t_2_ph_i = $t_1_i;
   }
   var $t_2_ph_i;
   var $438 = ($t_2_ph_i | 0) == 0;
   $_$125 : do {
    if ($438) {
     var $rsize_3_lcssa_i = $rsize_2_i;
     var $v_3_lcssa_i = $v_2_i;
    } else {
     var $t_224_i = $t_2_ph_i;
     var $rsize_325_i = $rsize_2_i;
     var $v_326_i = $v_2_i;
     while (1) {
      var $v_326_i;
      var $rsize_325_i;
      var $t_224_i;
      var $439 = $t_224_i + 4 | 0;
      var $440 = HEAP32[$439 >> 2];
      var $441 = $440 & -8;
      var $442 = $441 - $342 | 0;
      var $443 = $442 >>> 0 < $rsize_325_i >>> 0;
      var $_rsize_3_i = $443 ? $442 : $rsize_325_i;
      var $t_2_v_3_i = $443 ? $t_224_i : $v_326_i;
      var $444 = $t_224_i + 16 | 0;
      var $445 = HEAPU32[$444 >> 2];
      var $446 = ($445 | 0) == 0;
      if (!$446) {
       var $t_224_i = $445;
       var $rsize_325_i = $_rsize_3_i;
       var $v_326_i = $t_2_v_3_i;
       continue;
      }
      var $447 = $t_224_i + 20 | 0;
      var $448 = HEAPU32[$447 >> 2];
      var $449 = ($448 | 0) == 0;
      if ($449) {
       var $rsize_3_lcssa_i = $_rsize_3_i;
       var $v_3_lcssa_i = $t_2_v_3_i;
       break $_$125;
      }
      var $t_224_i = $448;
      var $rsize_325_i = $_rsize_3_i;
      var $v_326_i = $t_2_v_3_i;
     }
    }
   } while (0);
   var $v_3_lcssa_i;
   var $rsize_3_lcssa_i;
   var $450 = ($v_3_lcssa_i | 0) == 0;
   if ($450) {
    var $nb_0 = $342;
    __label__ = 155;
    break;
   }
   var $452 = HEAP32[(__gm_ + 8 | 0) >> 2];
   var $453 = $452 - $342 | 0;
   var $454 = $rsize_3_lcssa_i >>> 0 < $453 >>> 0;
   if (!$454) {
    var $nb_0 = $342;
    __label__ = 155;
    break;
   }
   var $456 = $v_3_lcssa_i;
   var $457 = HEAPU32[(__gm_ + 16 | 0) >> 2];
   var $458 = $456 >>> 0 < $457 >>> 0;
   do {
    if (!$458) {
     var $460 = $456 + $342 | 0;
     var $461 = $460;
     var $462 = $456 >>> 0 < $460 >>> 0;
     if (!$462) {
      break;
     }
     var $464 = $v_3_lcssa_i + 24 | 0;
     var $465 = HEAPU32[$464 >> 2];
     var $466 = $v_3_lcssa_i + 12 | 0;
     var $467 = HEAPU32[$466 >> 2];
     var $468 = ($467 | 0) == ($v_3_lcssa_i | 0);
     do {
      if ($468) {
       var $479 = $v_3_lcssa_i + 20 | 0;
       var $480 = HEAP32[$479 >> 2];
       var $481 = ($480 | 0) == 0;
       if ($481) {
        var $483 = $v_3_lcssa_i + 16 | 0;
        var $484 = HEAP32[$483 >> 2];
        var $485 = ($484 | 0) == 0;
        if ($485) {
         var $R_1_i19 = 0;
         break;
        }
        var $RP_0_i17 = $483;
        var $R_0_i16 = $484;
       } else {
        var $RP_0_i17 = $479;
        var $R_0_i16 = $480;
        __label__ = 103;
       }
       while (1) {
        var $R_0_i16;
        var $RP_0_i17;
        var $486 = $R_0_i16 + 20 | 0;
        var $487 = HEAP32[$486 >> 2];
        var $488 = ($487 | 0) == 0;
        if (!$488) {
         var $RP_0_i17 = $486;
         var $R_0_i16 = $487;
         continue;
        }
        var $490 = $R_0_i16 + 16 | 0;
        var $491 = HEAPU32[$490 >> 2];
        var $492 = ($491 | 0) == 0;
        if ($492) {
         break;
        }
        var $RP_0_i17 = $490;
        var $R_0_i16 = $491;
       }
       var $494 = $RP_0_i17;
       var $495 = $494 >>> 0 < $457 >>> 0;
       if ($495) {
        _abort();
       } else {
        HEAP32[$RP_0_i17 >> 2] = 0;
        var $R_1_i19 = $R_0_i16;
       }
      } else {
       var $470 = $v_3_lcssa_i + 8 | 0;
       var $471 = HEAPU32[$470 >> 2];
       var $472 = $471;
       var $473 = $472 >>> 0 < $457 >>> 0;
       if ($473) {
        _abort();
       } else {
        var $475 = $471 + 12 | 0;
        HEAP32[$475 >> 2] = $467;
        var $476 = $467 + 8 | 0;
        HEAP32[$476 >> 2] = $471;
        var $R_1_i19 = $467;
       }
      }
     } while (0);
     var $R_1_i19;
     var $499 = ($465 | 0) == 0;
     $_$151 : do {
      if (!$499) {
       var $501 = $v_3_lcssa_i + 28 | 0;
       var $502 = HEAP32[$501 >> 2];
       var $503 = __gm_ + 304 + ($502 << 2) | 0;
       var $504 = HEAP32[$503 >> 2];
       var $505 = ($v_3_lcssa_i | 0) == ($504 | 0);
       do {
        if ($505) {
         HEAP32[$503 >> 2] = $R_1_i19;
         var $cond_i20 = ($R_1_i19 | 0) == 0;
         if (!$cond_i20) {
          break;
         }
         var $507 = HEAP32[$501 >> 2];
         var $508 = 1 << $507;
         var $509 = $508 ^ -1;
         var $510 = HEAP32[(__gm_ + 4 | 0) >> 2];
         var $511 = $510 & $509;
         HEAP32[(__gm_ + 4 | 0) >> 2] = $511;
         break $_$151;
        }
        var $513 = $465;
        var $514 = HEAPU32[(__gm_ + 16 | 0) >> 2];
        var $515 = $513 >>> 0 < $514 >>> 0;
        if ($515) {
         _abort();
        } else {
         var $517 = $465 + 16 | 0;
         var $518 = HEAP32[$517 >> 2];
         var $519 = ($518 | 0) == ($v_3_lcssa_i | 0);
         if ($519) {
          HEAP32[$517 >> 2] = $R_1_i19;
         } else {
          var $522 = $465 + 20 | 0;
          HEAP32[$522 >> 2] = $R_1_i19;
         }
         var $525 = ($R_1_i19 | 0) == 0;
         if ($525) {
          break $_$151;
         }
        }
       } while (0);
       var $527 = $R_1_i19;
       var $528 = HEAPU32[(__gm_ + 16 | 0) >> 2];
       var $529 = $527 >>> 0 < $528 >>> 0;
       if ($529) {
        _abort();
       } else {
        var $531 = $R_1_i19 + 24 | 0;
        HEAP32[$531 >> 2] = $465;
        var $532 = $v_3_lcssa_i + 16 | 0;
        var $533 = HEAPU32[$532 >> 2];
        var $534 = ($533 | 0) == 0;
        if (!$534) {
         var $536 = $533;
         var $537 = HEAPU32[(__gm_ + 16 | 0) >> 2];
         var $538 = $536 >>> 0 < $537 >>> 0;
         if ($538) {
          _abort();
         } else {
          var $540 = $R_1_i19 + 16 | 0;
          HEAP32[$540 >> 2] = $533;
          var $541 = $533 + 24 | 0;
          HEAP32[$541 >> 2] = $R_1_i19;
         }
        }
        var $544 = $v_3_lcssa_i + 20 | 0;
        var $545 = HEAPU32[$544 >> 2];
        var $546 = ($545 | 0) == 0;
        if ($546) {
         break;
        }
        var $548 = $545;
        var $549 = HEAPU32[(__gm_ + 16 | 0) >> 2];
        var $550 = $548 >>> 0 < $549 >>> 0;
        if ($550) {
         _abort();
        } else {
         var $552 = $R_1_i19 + 20 | 0;
         HEAP32[$552 >> 2] = $545;
         var $553 = $545 + 24 | 0;
         HEAP32[$553 >> 2] = $R_1_i19;
        }
       }
      }
     } while (0);
     var $557 = $rsize_3_lcssa_i >>> 0 < 16;
     $_$179 : do {
      if ($557) {
       var $559 = $rsize_3_lcssa_i + $342 | 0;
       var $560 = $559 | 3;
       var $561 = $v_3_lcssa_i + 4 | 0;
       HEAP32[$561 >> 2] = $560;
       var $_sum18_i = $559 + 4 | 0;
       var $562 = $456 + $_sum18_i | 0;
       var $563 = $562;
       var $564 = HEAP32[$563 >> 2];
       var $565 = $564 | 1;
       HEAP32[$563 >> 2] = $565;
      } else {
       var $567 = $342 | 3;
       var $568 = $v_3_lcssa_i + 4 | 0;
       HEAP32[$568 >> 2] = $567;
       var $569 = $rsize_3_lcssa_i | 1;
       var $_sum_i2232 = $342 | 4;
       var $570 = $456 + $_sum_i2232 | 0;
       var $571 = $570;
       HEAP32[$571 >> 2] = $569;
       var $_sum1_i23 = $rsize_3_lcssa_i + $342 | 0;
       var $572 = $456 + $_sum1_i23 | 0;
       var $573 = $572;
       HEAP32[$573 >> 2] = $rsize_3_lcssa_i;
       var $574 = $rsize_3_lcssa_i >>> 0 < 256;
       if ($574) {
        var $576 = $rsize_3_lcssa_i >>> 3;
        var $577 = $rsize_3_lcssa_i >>> 2;
        var $578 = $577 & 1073741822;
        var $579 = __gm_ + 40 + ($578 << 2) | 0;
        var $580 = $579;
        var $581 = HEAPU32[(__gm_ | 0) >> 2];
        var $582 = 1 << $576;
        var $583 = $581 & $582;
        var $584 = ($583 | 0) == 0;
        do {
         if ($584) {
          var $586 = $581 | $582;
          HEAP32[(__gm_ | 0) >> 2] = $586;
          var $_sum14_pre_i = $578 + 2 | 0;
          var $_pre_i24 = __gm_ + 40 + ($_sum14_pre_i << 2) | 0;
          var $F5_0_i = $580;
          var $_pre_phi_i25 = $_pre_i24;
         } else {
          var $_sum17_i = $578 + 2 | 0;
          var $588 = __gm_ + 40 + ($_sum17_i << 2) | 0;
          var $589 = HEAPU32[$588 >> 2];
          var $590 = $589;
          var $591 = HEAPU32[(__gm_ + 16 | 0) >> 2];
          var $592 = $590 >>> 0 < $591 >>> 0;
          if (!$592) {
           var $F5_0_i = $589;
           var $_pre_phi_i25 = $588;
           break;
          }
          _abort();
         }
        } while (0);
        var $_pre_phi_i25;
        var $F5_0_i;
        HEAP32[$_pre_phi_i25 >> 2] = $461;
        var $595 = $F5_0_i + 12 | 0;
        HEAP32[$595 >> 2] = $461;
        var $_sum15_i = $342 + 8 | 0;
        var $596 = $456 + $_sum15_i | 0;
        var $597 = $596;
        HEAP32[$597 >> 2] = $F5_0_i;
        var $_sum16_i = $342 + 12 | 0;
        var $598 = $456 + $_sum16_i | 0;
        var $599 = $598;
        HEAP32[$599 >> 2] = $580;
       } else {
        var $601 = $460;
        var $602 = $rsize_3_lcssa_i >>> 8;
        var $603 = ($602 | 0) == 0;
        do {
         if ($603) {
          var $I7_0_i = 0;
         } else {
          var $605 = $rsize_3_lcssa_i >>> 0 > 16777215;
          if ($605) {
           var $I7_0_i = 31;
           break;
          }
          var $607 = $602 + 1048320 | 0;
          var $608 = $607 >>> 16;
          var $609 = $608 & 8;
          var $610 = $602 << $609;
          var $611 = $610 + 520192 | 0;
          var $612 = $611 >>> 16;
          var $613 = $612 & 4;
          var $614 = $610 << $613;
          var $615 = $614 + 245760 | 0;
          var $616 = $615 >>> 16;
          var $617 = $616 & 2;
          var $618 = $613 | $609;
          var $619 = $618 | $617;
          var $620 = 14 - $619 | 0;
          var $621 = $614 << $617;
          var $622 = $621 >>> 15;
          var $623 = $620 + $622 | 0;
          var $624 = $623 << 1;
          var $625 = $623 + 7 | 0;
          var $626 = $rsize_3_lcssa_i >>> ($625 >>> 0);
          var $627 = $626 & 1;
          var $628 = $627 | $624;
          var $I7_0_i = $628;
         }
        } while (0);
        var $I7_0_i;
        var $630 = __gm_ + 304 + ($I7_0_i << 2) | 0;
        var $_sum2_i = $342 + 28 | 0;
        var $631 = $456 + $_sum2_i | 0;
        var $632 = $631;
        HEAP32[$632 >> 2] = $I7_0_i;
        var $_sum3_i26 = $342 + 16 | 0;
        var $633 = $456 + $_sum3_i26 | 0;
        var $_sum4_i27 = $342 + 20 | 0;
        var $634 = $456 + $_sum4_i27 | 0;
        var $635 = $634;
        HEAP32[$635 >> 2] = 0;
        var $636 = $633;
        HEAP32[$636 >> 2] = 0;
        var $637 = HEAP32[(__gm_ + 4 | 0) >> 2];
        var $638 = 1 << $I7_0_i;
        var $639 = $637 & $638;
        var $640 = ($639 | 0) == 0;
        if ($640) {
         var $642 = $637 | $638;
         HEAP32[(__gm_ + 4 | 0) >> 2] = $642;
         HEAP32[$630 >> 2] = $601;
         var $643 = $630;
         var $_sum5_i = $342 + 24 | 0;
         var $644 = $456 + $_sum5_i | 0;
         var $645 = $644;
         HEAP32[$645 >> 2] = $643;
         var $_sum6_i = $342 + 12 | 0;
         var $646 = $456 + $_sum6_i | 0;
         var $647 = $646;
         HEAP32[$647 >> 2] = $601;
         var $_sum7_i = $342 + 8 | 0;
         var $648 = $456 + $_sum7_i | 0;
         var $649 = $648;
         HEAP32[$649 >> 2] = $601;
        } else {
         var $651 = HEAP32[$630 >> 2];
         var $652 = ($I7_0_i | 0) == 31;
         if ($652) {
          var $657 = 0;
         } else {
          var $654 = $I7_0_i >>> 1;
          var $655 = 25 - $654 | 0;
          var $657 = $655;
         }
         var $657;
         var $658 = $rsize_3_lcssa_i << $657;
         var $K12_0_i = $658;
         var $T_0_i = $651;
         while (1) {
          var $T_0_i;
          var $K12_0_i;
          var $660 = $T_0_i + 4 | 0;
          var $661 = HEAP32[$660 >> 2];
          var $662 = $661 & -8;
          var $663 = ($662 | 0) == ($rsize_3_lcssa_i | 0);
          if ($663) {
           var $683 = $T_0_i + 8 | 0;
           var $684 = HEAPU32[$683 >> 2];
           var $685 = $T_0_i;
           var $686 = HEAPU32[(__gm_ + 16 | 0) >> 2];
           var $687 = $685 >>> 0 < $686 >>> 0;
           do {
            if (!$687) {
             var $689 = $684;
             var $690 = $689 >>> 0 < $686 >>> 0;
             if ($690) {
              break;
             }
             var $692 = $684 + 12 | 0;
             HEAP32[$692 >> 2] = $601;
             HEAP32[$683 >> 2] = $601;
             var $_sum8_i = $342 + 8 | 0;
             var $693 = $456 + $_sum8_i | 0;
             var $694 = $693;
             HEAP32[$694 >> 2] = $684;
             var $_sum9_i = $342 + 12 | 0;
             var $695 = $456 + $_sum9_i | 0;
             var $696 = $695;
             HEAP32[$696 >> 2] = $T_0_i;
             var $_sum10_i = $342 + 24 | 0;
             var $697 = $456 + $_sum10_i | 0;
             var $698 = $697;
             HEAP32[$698 >> 2] = 0;
             break $_$179;
            }
           } while (0);
           _abort();
          } else {
           var $665 = $K12_0_i >>> 31;
           var $666 = $T_0_i + 16 + ($665 << 2) | 0;
           var $667 = HEAPU32[$666 >> 2];
           var $668 = ($667 | 0) == 0;
           var $669 = $K12_0_i << 1;
           if (!$668) {
            var $K12_0_i = $669;
            var $T_0_i = $667;
            continue;
           }
           var $671 = $666;
           var $672 = HEAPU32[(__gm_ + 16 | 0) >> 2];
           var $673 = $671 >>> 0 < $672 >>> 0;
           if (!$673) {
            HEAP32[$666 >> 2] = $601;
            var $_sum11_i = $342 + 24 | 0;
            var $675 = $456 + $_sum11_i | 0;
            var $676 = $675;
            HEAP32[$676 >> 2] = $T_0_i;
            var $_sum12_i = $342 + 12 | 0;
            var $677 = $456 + $_sum12_i | 0;
            var $678 = $677;
            HEAP32[$678 >> 2] = $601;
            var $_sum13_i = $342 + 8 | 0;
            var $679 = $456 + $_sum13_i | 0;
            var $680 = $679;
            HEAP32[$680 >> 2] = $601;
            break $_$179;
           }
           _abort();
          }
         }
        }
       }
      }
     } while (0);
     var $700 = $v_3_lcssa_i + 8 | 0;
     var $701 = $700;
     var $702 = ($700 | 0) == 0;
     if ($702) {
      var $nb_0 = $342;
      __label__ = 155;
      break $_$2;
     }
     var $mem_0 = $701;
     __label__ = 331;
     break $_$2;
    }
   } while (0);
   _abort();
  }
 } while (0);
 $_$215 : do {
  if (__label__ == 155) {
   var $nb_0;
   var $703 = HEAPU32[(__gm_ + 8 | 0) >> 2];
   var $704 = $nb_0 >>> 0 > $703 >>> 0;
   if ($704) {
    var $732 = HEAPU32[(__gm_ + 12 | 0) >> 2];
    var $733 = $nb_0 >>> 0 < $732 >>> 0;
    if ($733) {
     var $735 = $732 - $nb_0 | 0;
     HEAP32[(__gm_ + 12 | 0) >> 2] = $735;
     var $736 = HEAPU32[(__gm_ + 24 | 0) >> 2];
     var $737 = $736;
     var $738 = $737 + $nb_0 | 0;
     var $739 = $738;
     HEAP32[(__gm_ + 24 | 0) >> 2] = $739;
     var $740 = $735 | 1;
     var $_sum = $nb_0 + 4 | 0;
     var $741 = $737 + $_sum | 0;
     var $742 = $741;
     HEAP32[$742 >> 2] = $740;
     var $743 = $nb_0 | 3;
     var $744 = $736 + 4 | 0;
     HEAP32[$744 >> 2] = $743;
     var $745 = $736 + 8 | 0;
     var $746 = $745;
     var $mem_0 = $746;
    } else {
     var $748 = HEAP32[(_mparams | 0) >> 2];
     var $749 = ($748 | 0) == 0;
     do {
      if ($749) {
       var $751 = HEAP32[(_mparams | 0) >> 2];
       var $752 = ($751 | 0) == 0;
       if (!$752) {
        break;
       }
       var $754 = _sysconf(8);
       var $755 = $754 - 1 | 0;
       var $756 = $755 & $754;
       var $757 = ($756 | 0) == 0;
       if ($757) {
        HEAP32[(_mparams + 8 | 0) >> 2] = $754;
        HEAP32[(_mparams + 4 | 0) >> 2] = $754;
        HEAP32[(_mparams + 12 | 0) >> 2] = -1;
        HEAP32[(_mparams + 16 | 0) >> 2] = 2097152;
        HEAP32[(_mparams + 20 | 0) >> 2] = 0;
        HEAP32[(__gm_ + 440 | 0) >> 2] = 0;
        var $760 = _time(0);
        var $761 = $760 & -16;
        var $762 = $761 ^ 1431655768;
        HEAP32[(_mparams | 0) >> 2] = $762;
       } else {
        _abort();
       }
      }
     } while (0);
     var $763 = HEAP32[(__gm_ + 440 | 0) >> 2];
     var $764 = $763 & 4;
     var $765 = ($764 | 0) == 0;
     $_$234 : do {
      if ($765) {
       var $767 = HEAP32[(__gm_ + 24 | 0) >> 2];
       var $768 = ($767 | 0) == 0;
       $_$236 : do {
        if (!$768) {
         var $770 = $767;
         var $sp_0_i_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_0_i_i;
          var $772 = $sp_0_i_i | 0;
          var $773 = HEAPU32[$772 >> 2];
          var $774 = $773 >>> 0 > $770 >>> 0;
          if (!$774) {
           var $776 = $sp_0_i_i + 4 | 0;
           var $777 = HEAP32[$776 >> 2];
           var $778 = $773 + $777 | 0;
           var $779 = $778 >>> 0 > $770 >>> 0;
           if ($779) {
            break;
           }
          }
          var $781 = $sp_0_i_i + 8 | 0;
          var $782 = HEAPU32[$781 >> 2];
          var $783 = ($782 | 0) == 0;
          if ($783) {
           __label__ = 174;
           break $_$236;
          }
          var $sp_0_i_i = $782;
         }
         var $784 = ($sp_0_i_i | 0) == 0;
         if ($784) {
          __label__ = 174;
          break;
         }
         var $810 = HEAP32[(__gm_ + 12 | 0) >> 2];
         var $811 = HEAP32[(_mparams + 8 | 0) >> 2];
         var $812 = $nb_0 + 47 | 0;
         var $813 = $812 - $810 | 0;
         var $814 = $813 + $811 | 0;
         var $815 = -$811 | 0;
         var $816 = $814 & $815;
         var $817 = $816 >>> 0 < 2147483647;
         if (!$817) {
          var $tsize_0242932_ph_i = 0;
          __label__ = 189;
          break;
         }
         var $819 = _sbrk($816);
         var $820 = HEAP32[$772 >> 2];
         var $821 = HEAP32[$776 >> 2];
         var $822 = $820 + $821 | 0;
         var $823 = ($819 | 0) == ($822 | 0);
         var $_1_i = $823 ? $816 : 0;
         var $_2_i = $823 ? $819 : -1;
         var $tbase_0_i = $_2_i;
         var $tsize_0_i = $_1_i;
         var $asize_1_i = $816;
         var $br_0_i = $819;
         __label__ = 181;
         break;
        }
        __label__ = 174;
       } while (0);
       do {
        if (__label__ == 174) {
         var $785 = _sbrk(0);
         var $786 = ($785 | 0) == -1;
         if ($786) {
          var $tsize_0242932_ph_i = 0;
          __label__ = 189;
          break;
         }
         var $788 = HEAP32[(_mparams + 8 | 0) >> 2];
         var $789 = $nb_0 + 47 | 0;
         var $790 = $789 + $788 | 0;
         var $791 = -$788 | 0;
         var $792 = $790 & $791;
         var $793 = $785;
         var $794 = HEAP32[(_mparams + 4 | 0) >> 2];
         var $795 = $794 - 1 | 0;
         var $796 = $795 & $793;
         var $797 = ($796 | 0) == 0;
         if ($797) {
          var $asize_0_i = $792;
         } else {
          var $799 = $795 + $793 | 0;
          var $800 = -$794 | 0;
          var $801 = $799 & $800;
          var $802 = $792 - $793 | 0;
          var $803 = $802 + $801 | 0;
          var $asize_0_i = $803;
         }
         var $asize_0_i;
         var $805 = $asize_0_i >>> 0 < 2147483647;
         if (!$805) {
          var $tsize_0242932_ph_i = 0;
          __label__ = 189;
          break;
         }
         var $807 = _sbrk($asize_0_i);
         var $808 = ($807 | 0) == ($785 | 0);
         var $asize_0__i = $808 ? $asize_0_i : 0;
         var $__i = $808 ? $785 : -1;
         var $tbase_0_i = $__i;
         var $tsize_0_i = $asize_0__i;
         var $asize_1_i = $asize_0_i;
         var $br_0_i = $807;
         __label__ = 181;
         break;
        }
       } while (0);
       $_$253 : do {
        if (__label__ == 181) {
         var $br_0_i;
         var $asize_1_i;
         var $tsize_0_i;
         var $tbase_0_i;
         var $825 = -$asize_1_i | 0;
         var $826 = ($tbase_0_i | 0) == -1;
         if (!$826) {
          var $tsize_242_i = $tsize_0_i;
          var $tbase_243_i = $tbase_0_i;
          __label__ = 194;
          break $_$234;
         }
         var $828 = ($br_0_i | 0) != -1;
         var $829 = $asize_1_i >>> 0 < 2147483647;
         var $or_cond_i28 = $828 & $829;
         do {
          if ($or_cond_i28) {
           var $831 = $nb_0 + 48 | 0;
           var $832 = $asize_1_i >>> 0 < $831 >>> 0;
           if (!$832) {
            var $asize_2_i = $asize_1_i;
            break;
           }
           var $834 = HEAP32[(_mparams + 8 | 0) >> 2];
           var $835 = $nb_0 + 47 | 0;
           var $836 = $835 - $asize_1_i | 0;
           var $837 = $836 + $834 | 0;
           var $838 = -$834 | 0;
           var $839 = $837 & $838;
           var $840 = $839 >>> 0 < 2147483647;
           if (!$840) {
            var $asize_2_i = $asize_1_i;
            break;
           }
           var $842 = _sbrk($839);
           var $843 = ($842 | 0) == -1;
           if ($843) {
            var $847 = _sbrk($825);
            var $tsize_0242932_ph_i = $tsize_0_i;
            break $_$253;
           }
           var $845 = $839 + $asize_1_i | 0;
           var $asize_2_i = $845;
          } else {
           var $asize_2_i = $asize_1_i;
          }
         } while (0);
         var $asize_2_i;
         var $849 = ($br_0_i | 0) == -1;
         if (!$849) {
          var $tsize_242_i = $asize_2_i;
          var $tbase_243_i = $br_0_i;
          __label__ = 194;
          break $_$234;
         }
         var $852 = HEAP32[(__gm_ + 440 | 0) >> 2];
         var $853 = $852 | 4;
         HEAP32[(__gm_ + 440 | 0) >> 2] = $853;
         var $tsize_137_i = $tsize_0_i;
         __label__ = 191;
         break $_$234;
        }
       } while (0);
       var $tsize_0242932_ph_i;
       var $850 = HEAP32[(__gm_ + 440 | 0) >> 2];
       var $851 = $850 | 4;
       HEAP32[(__gm_ + 440 | 0) >> 2] = $851;
       var $tsize_137_i = $tsize_0242932_ph_i;
       __label__ = 191;
       break;
      }
      var $tsize_137_i = 0;
      __label__ = 191;
     } while (0);
     do {
      if (__label__ == 191) {
       var $tsize_137_i;
       var $854 = HEAP32[(_mparams + 8 | 0) >> 2];
       var $855 = $nb_0 + 47 | 0;
       var $856 = $855 + $854 | 0;
       var $857 = -$854 | 0;
       var $858 = $856 & $857;
       var $859 = $858 >>> 0 < 2147483647;
       if (!$859) {
        __label__ = 330;
        break;
       }
       var $861 = _sbrk($858);
       var $862 = _sbrk(0);
       var $notlhs_i = ($861 | 0) != -1;
       var $notrhs_i = ($862 | 0) != -1;
       var $or_cond3_not_i = $notrhs_i & $notlhs_i;
       var $863 = $861 >>> 0 < $862 >>> 0;
       var $or_cond4_i = $or_cond3_not_i & $863;
       if (!$or_cond4_i) {
        __label__ = 330;
        break;
       }
       var $864 = $862;
       var $865 = $861;
       var $866 = $864 - $865 | 0;
       var $867 = $nb_0 + 40 | 0;
       var $868 = $866 >>> 0 > $867 >>> 0;
       var $_tsize_1_i = $868 ? $866 : $tsize_137_i;
       var $_tbase_1_i = $868 ? $861 : -1;
       var $869 = ($_tbase_1_i | 0) == -1;
       if ($869) {
        __label__ = 330;
        break;
       }
       var $tsize_242_i = $_tsize_1_i;
       var $tbase_243_i = $_tbase_1_i;
       __label__ = 194;
       break;
      }
     } while (0);
     do {
      if (__label__ == 194) {
       var $tbase_243_i;
       var $tsize_242_i;
       var $870 = HEAP32[(__gm_ + 432 | 0) >> 2];
       var $871 = $870 + $tsize_242_i | 0;
       HEAP32[(__gm_ + 432 | 0) >> 2] = $871;
       var $872 = HEAPU32[(__gm_ + 436 | 0) >> 2];
       var $873 = $871 >>> 0 > $872 >>> 0;
       if ($873) {
        HEAP32[(__gm_ + 436 | 0) >> 2] = $871;
       }
       var $876 = HEAPU32[(__gm_ + 24 | 0) >> 2];
       var $877 = ($876 | 0) == 0;
       $_$275 : do {
        if ($877) {
         var $879 = HEAPU32[(__gm_ + 16 | 0) >> 2];
         var $880 = ($879 | 0) == 0;
         var $881 = $tbase_243_i >>> 0 < $879 >>> 0;
         var $or_cond5_i = $880 | $881;
         if ($or_cond5_i) {
          HEAP32[(__gm_ + 16 | 0) >> 2] = $tbase_243_i;
         }
         HEAP32[(__gm_ + 444 | 0) >> 2] = $tbase_243_i;
         HEAP32[(__gm_ + 448 | 0) >> 2] = $tsize_242_i;
         HEAP32[(__gm_ + 456 | 0) >> 2] = 0;
         var $884 = HEAP32[(_mparams | 0) >> 2];
         HEAP32[(__gm_ + 36 | 0) >> 2] = $884;
         HEAP32[(__gm_ + 32 | 0) >> 2] = -1;
         var $i_02_i_i = 0;
         while (1) {
          var $i_02_i_i;
          var $886 = $i_02_i_i << 1;
          var $887 = __gm_ + 40 + ($886 << 2) | 0;
          var $888 = $887;
          var $_sum_i_i = $886 + 3 | 0;
          var $889 = __gm_ + 40 + ($_sum_i_i << 2) | 0;
          HEAP32[$889 >> 2] = $888;
          var $_sum1_i_i = $886 + 2 | 0;
          var $890 = __gm_ + 40 + ($_sum1_i_i << 2) | 0;
          HEAP32[$890 >> 2] = $888;
          var $891 = $i_02_i_i + 1 | 0;
          var $exitcond_i_i = ($891 | 0) == 32;
          if ($exitcond_i_i) {
           break;
          }
          var $i_02_i_i = $891;
         }
         var $892 = $tbase_243_i + 8 | 0;
         var $893 = $892;
         var $894 = $893 & 7;
         var $895 = ($894 | 0) == 0;
         if ($895) {
          var $899 = 0;
         } else {
          var $897 = -$893 | 0;
          var $898 = $897 & 7;
          var $899 = $898;
         }
         var $899;
         var $900 = $tbase_243_i + $899 | 0;
         var $901 = $900;
         var $902 = $tsize_242_i - 40 | 0;
         var $903 = $902 - $899 | 0;
         HEAP32[(__gm_ + 24 | 0) >> 2] = $901;
         HEAP32[(__gm_ + 12 | 0) >> 2] = $903;
         var $904 = $903 | 1;
         var $_sum_i9_i = $899 + 4 | 0;
         var $905 = $tbase_243_i + $_sum_i9_i | 0;
         var $906 = $905;
         HEAP32[$906 >> 2] = $904;
         var $_sum2_i_i = $tsize_242_i - 36 | 0;
         var $907 = $tbase_243_i + $_sum2_i_i | 0;
         var $908 = $907;
         HEAP32[$908 >> 2] = 40;
         var $909 = HEAP32[(_mparams + 16 | 0) >> 2];
         HEAP32[(__gm_ + 28 | 0) >> 2] = $909;
        } else {
         var $sp_0_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_0_i;
          var $910 = ($sp_0_i | 0) == 0;
          if ($910) {
           break;
          }
          var $912 = $sp_0_i | 0;
          var $913 = HEAPU32[$912 >> 2];
          var $914 = $sp_0_i + 4 | 0;
          var $915 = HEAPU32[$914 >> 2];
          var $916 = $913 + $915 | 0;
          var $917 = ($tbase_243_i | 0) == ($916 | 0);
          if ($917) {
           var $921 = $sp_0_i + 12 | 0;
           var $922 = HEAP32[$921 >> 2];
           var $923 = $922 & 8;
           var $924 = ($923 | 0) == 0;
           if (!$924) {
            break;
           }
           var $926 = $876;
           var $927 = $926 >>> 0 >= $913 >>> 0;
           var $928 = $926 >>> 0 < $tbase_243_i >>> 0;
           var $or_cond44_i = $927 & $928;
           if (!$or_cond44_i) {
            break;
           }
           var $930 = $915 + $tsize_242_i | 0;
           HEAP32[$914 >> 2] = $930;
           var $931 = HEAP32[(__gm_ + 24 | 0) >> 2];
           var $932 = HEAP32[(__gm_ + 12 | 0) >> 2];
           var $933 = $932 + $tsize_242_i | 0;
           var $934 = $931;
           var $935 = $931 + 8 | 0;
           var $936 = $935;
           var $937 = $936 & 7;
           var $938 = ($937 | 0) == 0;
           if ($938) {
            var $942 = 0;
           } else {
            var $940 = -$936 | 0;
            var $941 = $940 & 7;
            var $942 = $941;
           }
           var $942;
           var $943 = $934 + $942 | 0;
           var $944 = $943;
           var $945 = $933 - $942 | 0;
           HEAP32[(__gm_ + 24 | 0) >> 2] = $944;
           HEAP32[(__gm_ + 12 | 0) >> 2] = $945;
           var $946 = $945 | 1;
           var $_sum_i13_i = $942 + 4 | 0;
           var $947 = $934 + $_sum_i13_i | 0;
           var $948 = $947;
           HEAP32[$948 >> 2] = $946;
           var $_sum2_i14_i = $933 + 4 | 0;
           var $949 = $934 + $_sum2_i14_i | 0;
           var $950 = $949;
           HEAP32[$950 >> 2] = 40;
           var $951 = HEAP32[(_mparams + 16 | 0) >> 2];
           HEAP32[(__gm_ + 28 | 0) >> 2] = $951;
           break $_$275;
          }
          var $919 = $sp_0_i + 8 | 0;
          var $920 = HEAP32[$919 >> 2];
          var $sp_0_i = $920;
         }
         var $952 = HEAPU32[(__gm_ + 16 | 0) >> 2];
         var $953 = $tbase_243_i >>> 0 < $952 >>> 0;
         if ($953) {
          HEAP32[(__gm_ + 16 | 0) >> 2] = $tbase_243_i;
         }
         var $955 = $tbase_243_i + $tsize_242_i | 0;
         var $sp_1_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_1_i;
          var $957 = ($sp_1_i | 0) == 0;
          if ($957) {
           __label__ = 293;
           break;
          }
          var $959 = $sp_1_i | 0;
          var $960 = HEAP32[$959 >> 2];
          var $961 = ($960 | 0) == ($955 | 0);
          if ($961) {
           __label__ = 218;
           break;
          }
          var $963 = $sp_1_i + 8 | 0;
          var $964 = HEAP32[$963 >> 2];
          var $sp_1_i = $964;
         }
         do {
          if (__label__ == 218) {
           var $965 = $sp_1_i + 12 | 0;
           var $966 = HEAP32[$965 >> 2];
           var $967 = $966 & 8;
           var $968 = ($967 | 0) == 0;
           if (!$968) {
            break;
           }
           HEAP32[$959 >> 2] = $tbase_243_i;
           var $970 = $sp_1_i + 4 | 0;
           var $971 = HEAP32[$970 >> 2];
           var $972 = $971 + $tsize_242_i | 0;
           HEAP32[$970 >> 2] = $972;
           var $973 = $tbase_243_i + 8 | 0;
           var $974 = $973;
           var $975 = $974 & 7;
           var $976 = ($975 | 0) == 0;
           if ($976) {
            var $981 = 0;
           } else {
            var $978 = -$974 | 0;
            var $979 = $978 & 7;
            var $981 = $979;
           }
           var $981;
           var $982 = $tbase_243_i + $981 | 0;
           var $_sum79_i = $tsize_242_i + 8 | 0;
           var $983 = $tbase_243_i + $_sum79_i | 0;
           var $984 = $983;
           var $985 = $984 & 7;
           var $986 = ($985 | 0) == 0;
           if ($986) {
            var $991 = 0;
           } else {
            var $988 = -$984 | 0;
            var $989 = $988 & 7;
            var $991 = $989;
           }
           var $991;
           var $_sum80_i = $991 + $tsize_242_i | 0;
           var $992 = $tbase_243_i + $_sum80_i | 0;
           var $993 = $992;
           var $994 = $992;
           var $995 = $982;
           var $996 = $994 - $995 | 0;
           var $_sum_i16_i = $981 + $nb_0 | 0;
           var $997 = $tbase_243_i + $_sum_i16_i | 0;
           var $998 = $997;
           var $999 = $996 - $nb_0 | 0;
           var $1000 = $nb_0 | 3;
           var $_sum1_i17_i = $981 + 4 | 0;
           var $1001 = $tbase_243_i + $_sum1_i17_i | 0;
           var $1002 = $1001;
           HEAP32[$1002 >> 2] = $1000;
           var $1003 = HEAP32[(__gm_ + 24 | 0) >> 2];
           var $1004 = ($993 | 0) == ($1003 | 0);
           $_$314 : do {
            if ($1004) {
             var $1006 = HEAP32[(__gm_ + 12 | 0) >> 2];
             var $1007 = $1006 + $999 | 0;
             HEAP32[(__gm_ + 12 | 0) >> 2] = $1007;
             HEAP32[(__gm_ + 24 | 0) >> 2] = $998;
             var $1008 = $1007 | 1;
             var $_sum42_i_i = $_sum_i16_i + 4 | 0;
             var $1009 = $tbase_243_i + $_sum42_i_i | 0;
             var $1010 = $1009;
             HEAP32[$1010 >> 2] = $1008;
            } else {
             var $1012 = HEAP32[(__gm_ + 20 | 0) >> 2];
             var $1013 = ($993 | 0) == ($1012 | 0);
             if ($1013) {
              var $1015 = HEAP32[(__gm_ + 8 | 0) >> 2];
              var $1016 = $1015 + $999 | 0;
              HEAP32[(__gm_ + 8 | 0) >> 2] = $1016;
              HEAP32[(__gm_ + 20 | 0) >> 2] = $998;
              var $1017 = $1016 | 1;
              var $_sum40_i_i = $_sum_i16_i + 4 | 0;
              var $1018 = $tbase_243_i + $_sum40_i_i | 0;
              var $1019 = $1018;
              HEAP32[$1019 >> 2] = $1017;
              var $_sum41_i_i = $1016 + $_sum_i16_i | 0;
              var $1020 = $tbase_243_i + $_sum41_i_i | 0;
              var $1021 = $1020;
              HEAP32[$1021 >> 2] = $1016;
             } else {
              var $_sum2_i18_i = $tsize_242_i + 4 | 0;
              var $_sum81_i = $_sum2_i18_i + $991 | 0;
              var $1023 = $tbase_243_i + $_sum81_i | 0;
              var $1024 = $1023;
              var $1025 = HEAPU32[$1024 >> 2];
              var $1026 = $1025 & 3;
              var $1027 = ($1026 | 0) == 1;
              if ($1027) {
               var $1029 = $1025 & -8;
               var $1030 = $1025 >>> 3;
               var $1031 = $1025 >>> 0 < 256;
               $_$322 : do {
                if ($1031) {
                 var $_sum3738_i_i = $991 | 8;
                 var $_sum91_i = $_sum3738_i_i + $tsize_242_i | 0;
                 var $1033 = $tbase_243_i + $_sum91_i | 0;
                 var $1034 = $1033;
                 var $1035 = HEAPU32[$1034 >> 2];
                 var $_sum39_i_i = $tsize_242_i + 12 | 0;
                 var $_sum92_i = $_sum39_i_i + $991 | 0;
                 var $1036 = $tbase_243_i + $_sum92_i | 0;
                 var $1037 = $1036;
                 var $1038 = HEAPU32[$1037 >> 2];
                 var $1039 = ($1035 | 0) == ($1038 | 0);
                 if ($1039) {
                  var $1041 = 1 << $1030;
                  var $1042 = $1041 ^ -1;
                  var $1043 = HEAP32[(__gm_ | 0) >> 2];
                  var $1044 = $1043 & $1042;
                  HEAP32[(__gm_ | 0) >> 2] = $1044;
                 } else {
                  var $1046 = $1025 >>> 2;
                  var $1047 = $1046 & 1073741822;
                  var $1048 = __gm_ + 40 + ($1047 << 2) | 0;
                  var $1049 = $1048;
                  var $1050 = ($1035 | 0) == ($1049 | 0);
                  do {
                   if ($1050) {
                    __label__ = 233;
                   } else {
                    var $1052 = $1035;
                    var $1053 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                    var $1054 = $1052 >>> 0 < $1053 >>> 0;
                    if ($1054) {
                     __label__ = 236;
                     break;
                    }
                    __label__ = 233;
                    break;
                   }
                  } while (0);
                  do {
                   if (__label__ == 233) {
                    var $1056 = ($1038 | 0) == ($1049 | 0);
                    if (!$1056) {
                     var $1058 = $1038;
                     var $1059 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                     var $1060 = $1058 >>> 0 < $1059 >>> 0;
                     if ($1060) {
                      break;
                     }
                    }
                    var $1061 = $1035 + 12 | 0;
                    HEAP32[$1061 >> 2] = $1038;
                    var $1062 = $1038 + 8 | 0;
                    HEAP32[$1062 >> 2] = $1035;
                    break $_$322;
                   }
                  } while (0);
                  _abort();
                 }
                } else {
                 var $1064 = $992;
                 var $_sum34_i_i = $991 | 24;
                 var $_sum82_i = $_sum34_i_i + $tsize_242_i | 0;
                 var $1065 = $tbase_243_i + $_sum82_i | 0;
                 var $1066 = $1065;
                 var $1067 = HEAPU32[$1066 >> 2];
                 var $_sum5_i_i = $tsize_242_i + 12 | 0;
                 var $_sum83_i = $_sum5_i_i + $991 | 0;
                 var $1068 = $tbase_243_i + $_sum83_i | 0;
                 var $1069 = $1068;
                 var $1070 = HEAPU32[$1069 >> 2];
                 var $1071 = ($1070 | 0) == ($1064 | 0);
                 do {
                  if ($1071) {
                   var $_sum67_i_i = $991 | 16;
                   var $_sum89_i = $_sum2_i18_i + $_sum67_i_i | 0;
                   var $1084 = $tbase_243_i + $_sum89_i | 0;
                   var $1085 = $1084;
                   var $1086 = HEAP32[$1085 >> 2];
                   var $1087 = ($1086 | 0) == 0;
                   if ($1087) {
                    var $_sum90_i = $_sum67_i_i + $tsize_242_i | 0;
                    var $1089 = $tbase_243_i + $_sum90_i | 0;
                    var $1090 = $1089;
                    var $1091 = HEAP32[$1090 >> 2];
                    var $1092 = ($1091 | 0) == 0;
                    if ($1092) {
                     var $R_1_i_i = 0;
                     break;
                    }
                    var $RP_0_i_i = $1090;
                    var $R_0_i_i = $1091;
                   } else {
                    var $RP_0_i_i = $1085;
                    var $R_0_i_i = $1086;
                    __label__ = 243;
                   }
                   while (1) {
                    var $R_0_i_i;
                    var $RP_0_i_i;
                    var $1093 = $R_0_i_i + 20 | 0;
                    var $1094 = HEAP32[$1093 >> 2];
                    var $1095 = ($1094 | 0) == 0;
                    if (!$1095) {
                     var $RP_0_i_i = $1093;
                     var $R_0_i_i = $1094;
                     continue;
                    }
                    var $1097 = $R_0_i_i + 16 | 0;
                    var $1098 = HEAPU32[$1097 >> 2];
                    var $1099 = ($1098 | 0) == 0;
                    if ($1099) {
                     break;
                    }
                    var $RP_0_i_i = $1097;
                    var $R_0_i_i = $1098;
                   }
                   var $1101 = $RP_0_i_i;
                   var $1102 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                   var $1103 = $1101 >>> 0 < $1102 >>> 0;
                   if ($1103) {
                    _abort();
                   } else {
                    HEAP32[$RP_0_i_i >> 2] = 0;
                    var $R_1_i_i = $R_0_i_i;
                   }
                  } else {
                   var $_sum3536_i_i = $991 | 8;
                   var $_sum84_i = $_sum3536_i_i + $tsize_242_i | 0;
                   var $1073 = $tbase_243_i + $_sum84_i | 0;
                   var $1074 = $1073;
                   var $1075 = HEAPU32[$1074 >> 2];
                   var $1076 = $1075;
                   var $1077 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                   var $1078 = $1076 >>> 0 < $1077 >>> 0;
                   if ($1078) {
                    _abort();
                   } else {
                    var $1080 = $1075 + 12 | 0;
                    HEAP32[$1080 >> 2] = $1070;
                    var $1081 = $1070 + 8 | 0;
                    HEAP32[$1081 >> 2] = $1075;
                    var $R_1_i_i = $1070;
                   }
                  }
                 } while (0);
                 var $R_1_i_i;
                 var $1107 = ($1067 | 0) == 0;
                 if ($1107) {
                  break;
                 }
                 var $_sum30_i_i = $tsize_242_i + 28 | 0;
                 var $_sum85_i = $_sum30_i_i + $991 | 0;
                 var $1109 = $tbase_243_i + $_sum85_i | 0;
                 var $1110 = $1109;
                 var $1111 = HEAP32[$1110 >> 2];
                 var $1112 = __gm_ + 304 + ($1111 << 2) | 0;
                 var $1113 = HEAP32[$1112 >> 2];
                 var $1114 = ($1064 | 0) == ($1113 | 0);
                 do {
                  if ($1114) {
                   HEAP32[$1112 >> 2] = $R_1_i_i;
                   var $cond_i_i = ($R_1_i_i | 0) == 0;
                   if (!$cond_i_i) {
                    break;
                   }
                   var $1116 = HEAP32[$1110 >> 2];
                   var $1117 = 1 << $1116;
                   var $1118 = $1117 ^ -1;
                   var $1119 = HEAP32[(__gm_ + 4 | 0) >> 2];
                   var $1120 = $1119 & $1118;
                   HEAP32[(__gm_ + 4 | 0) >> 2] = $1120;
                   break $_$322;
                  }
                  var $1122 = $1067;
                  var $1123 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                  var $1124 = $1122 >>> 0 < $1123 >>> 0;
                  if ($1124) {
                   _abort();
                  } else {
                   var $1126 = $1067 + 16 | 0;
                   var $1127 = HEAP32[$1126 >> 2];
                   var $1128 = ($1127 | 0) == ($1064 | 0);
                   if ($1128) {
                    HEAP32[$1126 >> 2] = $R_1_i_i;
                   } else {
                    var $1131 = $1067 + 20 | 0;
                    HEAP32[$1131 >> 2] = $R_1_i_i;
                   }
                   var $1134 = ($R_1_i_i | 0) == 0;
                   if ($1134) {
                    break $_$322;
                   }
                  }
                 } while (0);
                 var $1136 = $R_1_i_i;
                 var $1137 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                 var $1138 = $1136 >>> 0 < $1137 >>> 0;
                 if ($1138) {
                  _abort();
                 } else {
                  var $1140 = $R_1_i_i + 24 | 0;
                  HEAP32[$1140 >> 2] = $1067;
                  var $_sum3132_i_i = $991 | 16;
                  var $_sum86_i = $_sum3132_i_i + $tsize_242_i | 0;
                  var $1141 = $tbase_243_i + $_sum86_i | 0;
                  var $1142 = $1141;
                  var $1143 = HEAPU32[$1142 >> 2];
                  var $1144 = ($1143 | 0) == 0;
                  if (!$1144) {
                   var $1146 = $1143;
                   var $1147 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                   var $1148 = $1146 >>> 0 < $1147 >>> 0;
                   if ($1148) {
                    _abort();
                   } else {
                    var $1150 = $R_1_i_i + 16 | 0;
                    HEAP32[$1150 >> 2] = $1143;
                    var $1151 = $1143 + 24 | 0;
                    HEAP32[$1151 >> 2] = $R_1_i_i;
                   }
                  }
                  var $_sum87_i = $_sum2_i18_i + $_sum3132_i_i | 0;
                  var $1154 = $tbase_243_i + $_sum87_i | 0;
                  var $1155 = $1154;
                  var $1156 = HEAPU32[$1155 >> 2];
                  var $1157 = ($1156 | 0) == 0;
                  if ($1157) {
                   break;
                  }
                  var $1159 = $1156;
                  var $1160 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                  var $1161 = $1159 >>> 0 < $1160 >>> 0;
                  if ($1161) {
                   _abort();
                  } else {
                   var $1163 = $R_1_i_i + 20 | 0;
                   HEAP32[$1163 >> 2] = $1156;
                   var $1164 = $1156 + 24 | 0;
                   HEAP32[$1164 >> 2] = $R_1_i_i;
                  }
                 }
                }
               } while (0);
               var $_sum9_i_i = $1029 | $991;
               var $_sum88_i = $_sum9_i_i + $tsize_242_i | 0;
               var $1168 = $tbase_243_i + $_sum88_i | 0;
               var $1169 = $1168;
               var $1170 = $1029 + $999 | 0;
               var $oldfirst_0_i_i = $1169;
               var $qsize_0_i_i = $1170;
              } else {
               var $oldfirst_0_i_i = $993;
               var $qsize_0_i_i = $999;
              }
              var $qsize_0_i_i;
              var $oldfirst_0_i_i;
              var $1172 = $oldfirst_0_i_i + 4 | 0;
              var $1173 = HEAP32[$1172 >> 2];
              var $1174 = $1173 & -2;
              HEAP32[$1172 >> 2] = $1174;
              var $1175 = $qsize_0_i_i | 1;
              var $_sum10_i_i = $_sum_i16_i + 4 | 0;
              var $1176 = $tbase_243_i + $_sum10_i_i | 0;
              var $1177 = $1176;
              HEAP32[$1177 >> 2] = $1175;
              var $_sum11_i19_i = $qsize_0_i_i + $_sum_i16_i | 0;
              var $1178 = $tbase_243_i + $_sum11_i19_i | 0;
              var $1179 = $1178;
              HEAP32[$1179 >> 2] = $qsize_0_i_i;
              var $1180 = $qsize_0_i_i >>> 0 < 256;
              if ($1180) {
               var $1182 = $qsize_0_i_i >>> 3;
               var $1183 = $qsize_0_i_i >>> 2;
               var $1184 = $1183 & 1073741822;
               var $1185 = __gm_ + 40 + ($1184 << 2) | 0;
               var $1186 = $1185;
               var $1187 = HEAPU32[(__gm_ | 0) >> 2];
               var $1188 = 1 << $1182;
               var $1189 = $1187 & $1188;
               var $1190 = ($1189 | 0) == 0;
               do {
                if ($1190) {
                 var $1192 = $1187 | $1188;
                 HEAP32[(__gm_ | 0) >> 2] = $1192;
                 var $_sum26_pre_i_i = $1184 + 2 | 0;
                 var $_pre_i_i = __gm_ + 40 + ($_sum26_pre_i_i << 2) | 0;
                 var $F4_0_i_i = $1186;
                 var $_pre_phi_i20_i = $_pre_i_i;
                } else {
                 var $_sum29_i_i = $1184 + 2 | 0;
                 var $1194 = __gm_ + 40 + ($_sum29_i_i << 2) | 0;
                 var $1195 = HEAPU32[$1194 >> 2];
                 var $1196 = $1195;
                 var $1197 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                 var $1198 = $1196 >>> 0 < $1197 >>> 0;
                 if (!$1198) {
                  var $F4_0_i_i = $1195;
                  var $_pre_phi_i20_i = $1194;
                  break;
                 }
                 _abort();
                }
               } while (0);
               var $_pre_phi_i20_i;
               var $F4_0_i_i;
               HEAP32[$_pre_phi_i20_i >> 2] = $998;
               var $1201 = $F4_0_i_i + 12 | 0;
               HEAP32[$1201 >> 2] = $998;
               var $_sum27_i_i = $_sum_i16_i + 8 | 0;
               var $1202 = $tbase_243_i + $_sum27_i_i | 0;
               var $1203 = $1202;
               HEAP32[$1203 >> 2] = $F4_0_i_i;
               var $_sum28_i_i = $_sum_i16_i + 12 | 0;
               var $1204 = $tbase_243_i + $_sum28_i_i | 0;
               var $1205 = $1204;
               HEAP32[$1205 >> 2] = $1186;
              } else {
               var $1207 = $997;
               var $1208 = $qsize_0_i_i >>> 8;
               var $1209 = ($1208 | 0) == 0;
               do {
                if ($1209) {
                 var $I7_0_i_i = 0;
                } else {
                 var $1211 = $qsize_0_i_i >>> 0 > 16777215;
                 if ($1211) {
                  var $I7_0_i_i = 31;
                  break;
                 }
                 var $1213 = $1208 + 1048320 | 0;
                 var $1214 = $1213 >>> 16;
                 var $1215 = $1214 & 8;
                 var $1216 = $1208 << $1215;
                 var $1217 = $1216 + 520192 | 0;
                 var $1218 = $1217 >>> 16;
                 var $1219 = $1218 & 4;
                 var $1220 = $1216 << $1219;
                 var $1221 = $1220 + 245760 | 0;
                 var $1222 = $1221 >>> 16;
                 var $1223 = $1222 & 2;
                 var $1224 = $1219 | $1215;
                 var $1225 = $1224 | $1223;
                 var $1226 = 14 - $1225 | 0;
                 var $1227 = $1220 << $1223;
                 var $1228 = $1227 >>> 15;
                 var $1229 = $1226 + $1228 | 0;
                 var $1230 = $1229 << 1;
                 var $1231 = $1229 + 7 | 0;
                 var $1232 = $qsize_0_i_i >>> ($1231 >>> 0);
                 var $1233 = $1232 & 1;
                 var $1234 = $1233 | $1230;
                 var $I7_0_i_i = $1234;
                }
               } while (0);
               var $I7_0_i_i;
               var $1236 = __gm_ + 304 + ($I7_0_i_i << 2) | 0;
               var $_sum12_i_i = $_sum_i16_i + 28 | 0;
               var $1237 = $tbase_243_i + $_sum12_i_i | 0;
               var $1238 = $1237;
               HEAP32[$1238 >> 2] = $I7_0_i_i;
               var $_sum13_i_i = $_sum_i16_i + 16 | 0;
               var $1239 = $tbase_243_i + $_sum13_i_i | 0;
               var $_sum14_i_i = $_sum_i16_i + 20 | 0;
               var $1240 = $tbase_243_i + $_sum14_i_i | 0;
               var $1241 = $1240;
               HEAP32[$1241 >> 2] = 0;
               var $1242 = $1239;
               HEAP32[$1242 >> 2] = 0;
               var $1243 = HEAP32[(__gm_ + 4 | 0) >> 2];
               var $1244 = 1 << $I7_0_i_i;
               var $1245 = $1243 & $1244;
               var $1246 = ($1245 | 0) == 0;
               if ($1246) {
                var $1248 = $1243 | $1244;
                HEAP32[(__gm_ + 4 | 0) >> 2] = $1248;
                HEAP32[$1236 >> 2] = $1207;
                var $1249 = $1236;
                var $_sum15_i_i = $_sum_i16_i + 24 | 0;
                var $1250 = $tbase_243_i + $_sum15_i_i | 0;
                var $1251 = $1250;
                HEAP32[$1251 >> 2] = $1249;
                var $_sum16_i_i = $_sum_i16_i + 12 | 0;
                var $1252 = $tbase_243_i + $_sum16_i_i | 0;
                var $1253 = $1252;
                HEAP32[$1253 >> 2] = $1207;
                var $_sum17_i_i = $_sum_i16_i + 8 | 0;
                var $1254 = $tbase_243_i + $_sum17_i_i | 0;
                var $1255 = $1254;
                HEAP32[$1255 >> 2] = $1207;
               } else {
                var $1257 = HEAP32[$1236 >> 2];
                var $1258 = ($I7_0_i_i | 0) == 31;
                if ($1258) {
                 var $1263 = 0;
                } else {
                 var $1260 = $I7_0_i_i >>> 1;
                 var $1261 = 25 - $1260 | 0;
                 var $1263 = $1261;
                }
                var $1263;
                var $1264 = $qsize_0_i_i << $1263;
                var $K8_0_i_i = $1264;
                var $T_0_i21_i = $1257;
                while (1) {
                 var $T_0_i21_i;
                 var $K8_0_i_i;
                 var $1266 = $T_0_i21_i + 4 | 0;
                 var $1267 = HEAP32[$1266 >> 2];
                 var $1268 = $1267 & -8;
                 var $1269 = ($1268 | 0) == ($qsize_0_i_i | 0);
                 if ($1269) {
                  var $1289 = $T_0_i21_i + 8 | 0;
                  var $1290 = HEAPU32[$1289 >> 2];
                  var $1291 = $T_0_i21_i;
                  var $1292 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                  var $1293 = $1291 >>> 0 < $1292 >>> 0;
                  do {
                   if (!$1293) {
                    var $1295 = $1290;
                    var $1296 = $1295 >>> 0 < $1292 >>> 0;
                    if ($1296) {
                     break;
                    }
                    var $1298 = $1290 + 12 | 0;
                    HEAP32[$1298 >> 2] = $1207;
                    HEAP32[$1289 >> 2] = $1207;
                    var $_sum20_i_i = $_sum_i16_i + 8 | 0;
                    var $1299 = $tbase_243_i + $_sum20_i_i | 0;
                    var $1300 = $1299;
                    HEAP32[$1300 >> 2] = $1290;
                    var $_sum21_i_i = $_sum_i16_i + 12 | 0;
                    var $1301 = $tbase_243_i + $_sum21_i_i | 0;
                    var $1302 = $1301;
                    HEAP32[$1302 >> 2] = $T_0_i21_i;
                    var $_sum22_i_i = $_sum_i16_i + 24 | 0;
                    var $1303 = $tbase_243_i + $_sum22_i_i | 0;
                    var $1304 = $1303;
                    HEAP32[$1304 >> 2] = 0;
                    break $_$314;
                   }
                  } while (0);
                  _abort();
                 } else {
                  var $1271 = $K8_0_i_i >>> 31;
                  var $1272 = $T_0_i21_i + 16 + ($1271 << 2) | 0;
                  var $1273 = HEAPU32[$1272 >> 2];
                  var $1274 = ($1273 | 0) == 0;
                  var $1275 = $K8_0_i_i << 1;
                  if (!$1274) {
                   var $K8_0_i_i = $1275;
                   var $T_0_i21_i = $1273;
                   continue;
                  }
                  var $1277 = $1272;
                  var $1278 = HEAPU32[(__gm_ + 16 | 0) >> 2];
                  var $1279 = $1277 >>> 0 < $1278 >>> 0;
                  if (!$1279) {
                   HEAP32[$1272 >> 2] = $1207;
                   var $_sum23_i_i = $_sum_i16_i + 24 | 0;
                   var $1281 = $tbase_243_i + $_sum23_i_i | 0;
                   var $1282 = $1281;
                   HEAP32[$1282 >> 2] = $T_0_i21_i;
                   var $_sum24_i_i = $_sum_i16_i + 12 | 0;
                   var $1283 = $tbase_243_i + $_sum24_i_i | 0;
                   var $1284 = $1283;
                   HEAP32[$1284 >> 2] = $1207;
                   var $_sum25_i_i = $_sum_i16_i + 8 | 0;
                   var $1285 = $tbase_243_i + $_sum25_i_i | 0;
                   var $1286 = $1285;
                   HEAP32[$1286 >> 2] = $1207;
                   break $_$314;
                  }
                  _abort();
                 }
                }
               }
              }
             }
            }
           } while (0);
           var $_sum1819_i_i = $981 | 8;
           var $1305 = $tbase_243_i + $_sum1819_i_i | 0;
           var $mem_0 = $1305;
           break $_$215;
          }
         } while (0);
         var $1306 = $876;
         var $sp_0_i_i_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_0_i_i_i;
          var $1308 = $sp_0_i_i_i | 0;
          var $1309 = HEAPU32[$1308 >> 2];
          var $1310 = $1309 >>> 0 > $1306 >>> 0;
          if (!$1310) {
           var $1312 = $sp_0_i_i_i + 4 | 0;
           var $1313 = HEAPU32[$1312 >> 2];
           var $1314 = $1309 + $1313 | 0;
           var $1315 = $1314 >>> 0 > $1306 >>> 0;
           if ($1315) {
            var $1321 = $1309;
            var $1320 = $1313;
            break;
           }
          }
          var $1317 = $sp_0_i_i_i + 8 | 0;
          var $1318 = HEAPU32[$1317 >> 2];
          var $1319 = ($1318 | 0) == 0;
          if (!$1319) {
           var $sp_0_i_i_i = $1318;
           continue;
          }
          var $_pre14_i_i = 4;
          var $1321 = 0;
          var $1320 = $_pre14_i_i;
          break;
         }
         var $1320;
         var $1321;
         var $1322 = $1321 + $1320 | 0;
         var $_sum1_i10_i = $1320 - 39 | 0;
         var $1323 = $1321 + $_sum1_i10_i | 0;
         var $1324 = $1323;
         var $1325 = $1324 & 7;
         var $1326 = ($1325 | 0) == 0;
         if ($1326) {
          var $1331 = 0;
         } else {
          var $1328 = -$1324 | 0;
          var $1329 = $1328 & 7;
          var $1331 = $1329;
         }
         var $1331;
         var $_sum_i11_i = $1320 - 47 | 0;
         var $_sum2_i12_i = $_sum_i11_i + $1331 | 0;
         var $1332 = $1321 + $_sum2_i12_i | 0;
         var $1333 = $876 + 16 | 0;
         var $1334 = $1333;
         var $1335 = $1332 >>> 0 < $1334 >>> 0;
         var $1336 = $1335 ? $1306 : $1332;
         var $1337 = $1336 + 8 | 0;
         var $1338 = $1337;
         var $1339 = $tbase_243_i + 8 | 0;
         var $1340 = $1339;
         var $1341 = $1340 & 7;
         var $1342 = ($1341 | 0) == 0;
         if ($1342) {
          var $1346 = 0;
         } else {
          var $1344 = -$1340 | 0;
          var $1345 = $1344 & 7;
          var $1346 = $1345;
         }
         var $1346;
         var $1347 = $tbase_243_i + $1346 | 0;
         var $1348 = $1347;
         var $1349 = $tsize_242_i - 40 | 0;
         var $1350 = $1349 - $1346 | 0;
         HEAP32[(__gm_ + 24 | 0) >> 2] = $1348;
         HEAP32[(__gm_ + 12 | 0) >> 2] = $1350;
         var $1351 = $1350 | 1;
         var $_sum_i_i_i = $1346 + 4 | 0;
         var $1352 = $tbase_243_i + $_sum_i_i_i | 0;
         var $1353 = $1352;
         HEAP32[$1353 >> 2] = $1351;
         var $_sum2_i_i_i = $tsize_242_i - 36 | 0;
         var $1354 = $tbase_243_i + $_sum2_i_i_i | 0;
         var $1355 = $1354;
         HEAP32[$1355 >> 2] = 40;
         var $1356 = HEAP32[(_mparams + 16 | 0) >> 2];
         HEAP32[(__gm_ + 28 | 0) >> 2] = $1356;
         var $1357 = $1336 + 4 | 0;
         var $1358 = $1357;
         HEAP32[$1358 >> 2] = 27;
         HEAP32[$1337 >> 2] = HEAP32[(__gm_ + 444 | 0) >> 2];
         HEAP32[$1337 + 4 >> 2] = HEAP32[(__gm_ + 444 | 0) + 4 >> 2];
         HEAP32[$1337 + 8 >> 2] = HEAP32[(__gm_ + 444 | 0) + 8 >> 2];
         HEAP32[$1337 + 12 >> 2] = HEAP32[(__gm_ + 444 | 0) + 12 >> 2];
         HEAP32[(__gm_ + 444 | 0) >> 2] = $tbase_243_i;
         HEAP32[(__gm_ + 448 | 0) >> 2] = $tsize_242_i;
         HEAP32[(__gm_ + 456 | 0) >> 2] = 0;
         HEAP32[(__gm_ + 452 | 0) >> 2] = $1338;
         var $1359 = $1336 + 28 | 0;
         var $1360 = $1359;
         HEAP32[$1360 >> 2] = 7;
         var $1361 = $1336 + 32 | 0;
         var $1362 = $1361 >>> 0 < $1322 >>> 0;
         $_$426 : do {
          if ($1362) {
           var $1363 = $1360;
           while (1) {
            var $1363;
            var $1364 = $1363 + 4 | 0;
            HEAP32[$1364 >> 2] = 7;
            var $1365 = $1363 + 8 | 0;
            var $1366 = $1365;
            var $1367 = $1366 >>> 0 < $1322 >>> 0;
            if (!$1367) {
             break $_$426;
            }
            var $1363 = $1364;
           }
          }
         } while (0);
         var $1368 = ($1336 | 0) == ($1306 | 0);
         if ($1368) {
          break;
         }
         var $1370 = $1336;
         var $1371 = $876;
         var $1372 = $1370 - $1371 | 0;
         var $1373 = $1306 + $1372 | 0;
         var $_sum3_i_i = $1372 + 4 | 0;
         var $1374 = $1306 + $_sum3_i_i | 0;
         var $1375 = $1374;
         var $1376 = HEAP32[$1375 >> 2];
         var $1377 = $1376 & -2;
         HEAP32[$1375 >> 2] = $1377;
         var $1378 = $1372 | 1;
         var $1379 = $876 + 4 | 0;
         HEAP32[$1379 >> 2] = $1378;
         var $1380 = $1373;
         HEAP32[$1380 >> 2] = $1372;
         var $1381 = $1372 >>> 0 < 256;
         if ($1381) {
          var $1383 = $1372 >>> 3;
          var $1384 = $1372 >>> 2;
          var $1385 = $1384 & 1073741822;
          var $1386 = __gm_ + 40 + ($1385 << 2) | 0;
          var $1387 = $1386;
          var $1388 = HEAPU32[(__gm_ | 0) >> 2];
          var $1389 = 1 << $1383;
          var $1390 = $1388 & $1389;
          var $1391 = ($1390 | 0) == 0;
          do {
           if ($1391) {
            var $1393 = $1388 | $1389;
            HEAP32[(__gm_ | 0) >> 2] = $1393;
            var $_sum10_pre_i_i = $1385 + 2 | 0;
            var $_pre15_i_i = __gm_ + 40 + ($_sum10_pre_i_i << 2) | 0;
            var $F_0_i_i = $1387;
            var $_pre_phi_i_i = $_pre15_i_i;
           } else {
            var $_sum11_i_i = $1385 + 2 | 0;
            var $1395 = __gm_ + 40 + ($_sum11_i_i << 2) | 0;
            var $1396 = HEAPU32[$1395 >> 2];
            var $1397 = $1396;
            var $1398 = HEAPU32[(__gm_ + 16 | 0) >> 2];
            var $1399 = $1397 >>> 0 < $1398 >>> 0;
            if (!$1399) {
             var $F_0_i_i = $1396;
             var $_pre_phi_i_i = $1395;
             break;
            }
            _abort();
           }
          } while (0);
          var $_pre_phi_i_i;
          var $F_0_i_i;
          HEAP32[$_pre_phi_i_i >> 2] = $876;
          var $1402 = $F_0_i_i + 12 | 0;
          HEAP32[$1402 >> 2] = $876;
          var $1403 = $876 + 8 | 0;
          HEAP32[$1403 >> 2] = $F_0_i_i;
          var $1404 = $876 + 12 | 0;
          HEAP32[$1404 >> 2] = $1387;
         } else {
          var $1406 = $876;
          var $1407 = $1372 >>> 8;
          var $1408 = ($1407 | 0) == 0;
          do {
           if ($1408) {
            var $I1_0_i_i = 0;
           } else {
            var $1410 = $1372 >>> 0 > 16777215;
            if ($1410) {
             var $I1_0_i_i = 31;
             break;
            }
            var $1412 = $1407 + 1048320 | 0;
            var $1413 = $1412 >>> 16;
            var $1414 = $1413 & 8;
            var $1415 = $1407 << $1414;
            var $1416 = $1415 + 520192 | 0;
            var $1417 = $1416 >>> 16;
            var $1418 = $1417 & 4;
            var $1419 = $1415 << $1418;
            var $1420 = $1419 + 245760 | 0;
            var $1421 = $1420 >>> 16;
            var $1422 = $1421 & 2;
            var $1423 = $1418 | $1414;
            var $1424 = $1423 | $1422;
            var $1425 = 14 - $1424 | 0;
            var $1426 = $1419 << $1422;
            var $1427 = $1426 >>> 15;
            var $1428 = $1425 + $1427 | 0;
            var $1429 = $1428 << 1;
            var $1430 = $1428 + 7 | 0;
            var $1431 = $1372 >>> ($1430 >>> 0);
            var $1432 = $1431 & 1;
            var $1433 = $1432 | $1429;
            var $I1_0_i_i = $1433;
           }
          } while (0);
          var $I1_0_i_i;
          var $1435 = __gm_ + 304 + ($I1_0_i_i << 2) | 0;
          var $1436 = $876 + 28 | 0;
          var $I1_0_c_i_i = $I1_0_i_i;
          HEAP32[$1436 >> 2] = $I1_0_c_i_i;
          var $1437 = $876 + 20 | 0;
          HEAP32[$1437 >> 2] = 0;
          var $1438 = $876 + 16 | 0;
          HEAP32[$1438 >> 2] = 0;
          var $1439 = HEAP32[(__gm_ + 4 | 0) >> 2];
          var $1440 = 1 << $I1_0_i_i;
          var $1441 = $1439 & $1440;
          var $1442 = ($1441 | 0) == 0;
          if ($1442) {
           var $1444 = $1439 | $1440;
           HEAP32[(__gm_ + 4 | 0) >> 2] = $1444;
           HEAP32[$1435 >> 2] = $1406;
           var $1445 = $876 + 24 | 0;
           var $_c_i_i = $1435;
           HEAP32[$1445 >> 2] = $_c_i_i;
           var $1446 = $876 + 12 | 0;
           HEAP32[$1446 >> 2] = $876;
           var $1447 = $876 + 8 | 0;
           HEAP32[$1447 >> 2] = $876;
          } else {
           var $1449 = HEAP32[$1435 >> 2];
           var $1450 = ($I1_0_i_i | 0) == 31;
           if ($1450) {
            var $1455 = 0;
           } else {
            var $1452 = $I1_0_i_i >>> 1;
            var $1453 = 25 - $1452 | 0;
            var $1455 = $1453;
           }
           var $1455;
           var $1456 = $1372 << $1455;
           var $K2_0_i_i = $1456;
           var $T_0_i_i = $1449;
           while (1) {
            var $T_0_i_i;
            var $K2_0_i_i;
            var $1458 = $T_0_i_i + 4 | 0;
            var $1459 = HEAP32[$1458 >> 2];
            var $1460 = $1459 & -8;
            var $1461 = ($1460 | 0) == ($1372 | 0);
            if ($1461) {
             var $1478 = $T_0_i_i + 8 | 0;
             var $1479 = HEAPU32[$1478 >> 2];
             var $1480 = $T_0_i_i;
             var $1481 = HEAPU32[(__gm_ + 16 | 0) >> 2];
             var $1482 = $1480 >>> 0 < $1481 >>> 0;
             do {
              if (!$1482) {
               var $1484 = $1479;
               var $1485 = $1484 >>> 0 < $1481 >>> 0;
               if ($1485) {
                break;
               }
               var $1487 = $1479 + 12 | 0;
               HEAP32[$1487 >> 2] = $1406;
               HEAP32[$1478 >> 2] = $1406;
               var $1488 = $876 + 8 | 0;
               var $_c6_i_i = $1479;
               HEAP32[$1488 >> 2] = $_c6_i_i;
               var $1489 = $876 + 12 | 0;
               var $T_0_c_i_i = $T_0_i_i;
               HEAP32[$1489 >> 2] = $T_0_c_i_i;
               var $1490 = $876 + 24 | 0;
               HEAP32[$1490 >> 2] = 0;
               break $_$275;
              }
             } while (0);
             _abort();
            } else {
             var $1463 = $K2_0_i_i >>> 31;
             var $1464 = $T_0_i_i + 16 + ($1463 << 2) | 0;
             var $1465 = HEAPU32[$1464 >> 2];
             var $1466 = ($1465 | 0) == 0;
             var $1467 = $K2_0_i_i << 1;
             if (!$1466) {
              var $K2_0_i_i = $1467;
              var $T_0_i_i = $1465;
              continue;
             }
             var $1469 = $1464;
             var $1470 = HEAPU32[(__gm_ + 16 | 0) >> 2];
             var $1471 = $1469 >>> 0 < $1470 >>> 0;
             if (!$1471) {
              HEAP32[$1464 >> 2] = $1406;
              var $1473 = $876 + 24 | 0;
              var $T_0_c7_i_i = $T_0_i_i;
              HEAP32[$1473 >> 2] = $T_0_c7_i_i;
              var $1474 = $876 + 12 | 0;
              HEAP32[$1474 >> 2] = $876;
              var $1475 = $876 + 8 | 0;
              HEAP32[$1475 >> 2] = $876;
              break $_$275;
             }
             _abort();
            }
           }
          }
         }
        }
       } while (0);
       var $1491 = HEAPU32[(__gm_ + 12 | 0) >> 2];
       var $1492 = $1491 >>> 0 > $nb_0 >>> 0;
       if (!$1492) {
        break;
       }
       var $1494 = $1491 - $nb_0 | 0;
       HEAP32[(__gm_ + 12 | 0) >> 2] = $1494;
       var $1495 = HEAPU32[(__gm_ + 24 | 0) >> 2];
       var $1496 = $1495;
       var $1497 = $1496 + $nb_0 | 0;
       var $1498 = $1497;
       HEAP32[(__gm_ + 24 | 0) >> 2] = $1498;
       var $1499 = $1494 | 1;
       var $_sum_i30 = $nb_0 + 4 | 0;
       var $1500 = $1496 + $_sum_i30 | 0;
       var $1501 = $1500;
       HEAP32[$1501 >> 2] = $1499;
       var $1502 = $nb_0 | 3;
       var $1503 = $1495 + 4 | 0;
       HEAP32[$1503 >> 2] = $1502;
       var $1504 = $1495 + 8 | 0;
       var $1505 = $1504;
       var $mem_0 = $1505;
       break $_$215;
      }
     } while (0);
     var $1506 = ___errno();
     HEAP32[$1506 >> 2] = 12;
     var $mem_0 = 0;
    }
   } else {
    var $706 = $703 - $nb_0 | 0;
    var $707 = HEAPU32[(__gm_ + 20 | 0) >> 2];
    var $708 = $706 >>> 0 > 15;
    if ($708) {
     var $710 = $707;
     var $711 = $710 + $nb_0 | 0;
     var $712 = $711;
     HEAP32[(__gm_ + 20 | 0) >> 2] = $712;
     HEAP32[(__gm_ + 8 | 0) >> 2] = $706;
     var $713 = $706 | 1;
     var $_sum2 = $nb_0 + 4 | 0;
     var $714 = $710 + $_sum2 | 0;
     var $715 = $714;
     HEAP32[$715 >> 2] = $713;
     var $716 = $710 + $703 | 0;
     var $717 = $716;
     HEAP32[$717 >> 2] = $706;
     var $718 = $nb_0 | 3;
     var $719 = $707 + 4 | 0;
     HEAP32[$719 >> 2] = $718;
    } else {
     HEAP32[(__gm_ + 8 | 0) >> 2] = 0;
     HEAP32[(__gm_ + 20 | 0) >> 2] = 0;
     var $721 = $703 | 3;
     var $722 = $707 + 4 | 0;
     HEAP32[$722 >> 2] = $721;
     var $723 = $707;
     var $_sum1 = $703 + 4 | 0;
     var $724 = $723 + $_sum1 | 0;
     var $725 = $724;
     var $726 = HEAP32[$725 >> 2];
     var $727 = $726 | 1;
     HEAP32[$725 >> 2] = $727;
    }
    var $729 = $707 + 8 | 0;
    var $730 = $729;
    var $mem_0 = $730;
   }
  }
 } while (0);
 var $mem_0;
 return $mem_0;
 return null;
}
function _mallocNoU($bytes) {
 var __label__;
 var $1 = $bytes >>> 0 < 245;
 $_$2 : do {
  if ($1) {
   var $3 = $bytes >>> 0 < 11;
   if ($3) {
    var $8 = 16;
   } else {
    var $5 = $bytes + 11 | 0;
    var $6 = $5 & -8;
    var $8 = $6;
   }
   var $8;
   var $9 = $8 >>> 3;
   var $10 = HEAP32[(__gm_ | 0) >> 2];
   var $11 = $10 >>> ($9 >>> 0);
   var $12 = $11 & 3;
   var $13 = ($12 | 0) == 0;
   if (!$13) {
    var $15 = $11 & 1;
    var $16 = $15 ^ 1;
    var $17 = $16 + $9 | 0;
    var $18 = $17 << 1;
    var $19 = __gm_ + 40 + ($18 << 2) | 0;
    var $20 = $19;
    var $_sum10 = $18 + 2 | 0;
    var $21 = __gm_ + 40 + ($_sum10 << 2) | 0;
    var $22 = HEAP32[$21 >> 2];
    var $23 = $22 + 8 | 0;
    var $24 = HEAP32[$23 >> 2];
    var $25 = ($20 | 0) == ($24 | 0);
    if ($25) {
     var $27 = 1 << $17;
     var $28 = $27 ^ -1;
     var $29 = $10 & $28;
     HEAP32[(__gm_ | 0) >> 2] = $29;
    } else {
     var $31 = $24;
     var $32 = HEAP32[(__gm_ + 16 | 0) >> 2];
     var $33 = $31 >>> 0 < $32 >>> 0;
     if ($33) {
      _abort();
     } else {
      HEAP32[$21 >> 2] = $24;
      var $35 = $24 + 12 | 0;
      HEAP32[$35 >> 2] = $20;
     }
    }
    var $38 = $17 << 3;
    var $39 = $38 | 3;
    var $40 = $22 + 4 | 0;
    HEAP32[$40 >> 2] = $39;
    var $41 = $22;
    var $_sum1112 = $38 | 4;
    var $42 = $41 + $_sum1112 | 0;
    var $43 = $42;
    var $44 = HEAP32[$43 >> 2];
    var $45 = $44 | 1;
    HEAP32[$43 >> 2] = $45;
    var $46 = $23;
    var $mem_0 = $46;
    __label__ = 331;
    break;
   }
   var $48 = HEAP32[(__gm_ + 8 | 0) >> 2];
   var $49 = $8 >>> 0 > $48 >>> 0;
   if (!$49) {
    var $nb_0 = $8;
    __label__ = 155;
    break;
   }
   var $51 = ($11 | 0) == 0;
   if (!$51) {
    var $53 = $11 << $9;
    var $54 = 2 << $9;
    var $55 = -$54 | 0;
    var $56 = $54 | $55;
    var $57 = $53 & $56;
    var $58 = -$57 | 0;
    var $59 = $57 & $58;
    var $60 = $59 - 1 | 0;
    var $61 = $60 >>> 12;
    var $62 = $61 & 16;
    var $63 = $60 >>> ($62 >>> 0);
    var $64 = $63 >>> 5;
    var $65 = $64 & 8;
    var $66 = $63 >>> ($65 >>> 0);
    var $67 = $66 >>> 2;
    var $68 = $67 & 4;
    var $69 = $66 >>> ($68 >>> 0);
    var $70 = $69 >>> 1;
    var $71 = $70 & 2;
    var $72 = $69 >>> ($71 >>> 0);
    var $73 = $72 >>> 1;
    var $74 = $73 & 1;
    var $75 = $65 | $62;
    var $76 = $75 | $68;
    var $77 = $76 | $71;
    var $78 = $77 | $74;
    var $79 = $72 >>> ($74 >>> 0);
    var $80 = $78 + $79 | 0;
    var $81 = $80 << 1;
    var $82 = __gm_ + 40 + ($81 << 2) | 0;
    var $83 = $82;
    var $_sum4 = $81 + 2 | 0;
    var $84 = __gm_ + 40 + ($_sum4 << 2) | 0;
    var $85 = HEAP32[$84 >> 2];
    var $86 = $85 + 8 | 0;
    var $87 = HEAP32[$86 >> 2];
    var $88 = ($83 | 0) == ($87 | 0);
    if ($88) {
     var $90 = 1 << $80;
     var $91 = $90 ^ -1;
     var $92 = $10 & $91;
     HEAP32[(__gm_ | 0) >> 2] = $92;
    } else {
     var $94 = $87;
     var $95 = HEAP32[(__gm_ + 16 | 0) >> 2];
     var $96 = $94 >>> 0 < $95 >>> 0;
     if ($96) {
      _abort();
     } else {
      HEAP32[$84 >> 2] = $87;
      var $98 = $87 + 12 | 0;
      HEAP32[$98 >> 2] = $83;
     }
    }
    var $101 = $80 << 3;
    var $102 = $101 - $8 | 0;
    var $103 = $8 | 3;
    var $104 = $85 + 4 | 0;
    HEAP32[$104 >> 2] = $103;
    var $105 = $85;
    var $106 = $105 + $8 | 0;
    var $107 = $106;
    var $108 = $102 | 1;
    var $_sum56 = $8 | 4;
    var $109 = $105 + $_sum56 | 0;
    var $110 = $109;
    HEAP32[$110 >> 2] = $108;
    var $111 = $105 + $101 | 0;
    var $112 = $111;
    HEAP32[$112 >> 2] = $102;
    var $113 = HEAP32[(__gm_ + 8 | 0) >> 2];
    var $114 = ($113 | 0) == 0;
    if (!$114) {
     var $116 = HEAP32[(__gm_ + 20 | 0) >> 2];
     var $117 = $113 >>> 3;
     var $118 = $113 >>> 2;
     var $119 = $118 & 1073741822;
     var $120 = __gm_ + 40 + ($119 << 2) | 0;
     var $121 = $120;
     var $122 = HEAP32[(__gm_ | 0) >> 2];
     var $123 = 1 << $117;
     var $124 = $122 & $123;
     var $125 = ($124 | 0) == 0;
     do {
      if ($125) {
       var $127 = $122 | $123;
       HEAP32[(__gm_ | 0) >> 2] = $127;
       var $_sum8_pre = $119 + 2 | 0;
       var $_pre = __gm_ + 40 + ($_sum8_pre << 2) | 0;
       var $F4_0 = $121;
       var $_pre_phi = $_pre;
      } else {
       var $_sum9 = $119 + 2 | 0;
       var $129 = __gm_ + 40 + ($_sum9 << 2) | 0;
       var $130 = HEAP32[$129 >> 2];
       var $131 = $130;
       var $132 = HEAP32[(__gm_ + 16 | 0) >> 2];
       var $133 = $131 >>> 0 < $132 >>> 0;
       if (!$133) {
        var $F4_0 = $130;
        var $_pre_phi = $129;
        break;
       }
       _abort();
      }
     } while (0);
     var $_pre_phi;
     var $F4_0;
     HEAP32[$_pre_phi >> 2] = $116;
     var $136 = $F4_0 + 12 | 0;
     HEAP32[$136 >> 2] = $116;
     var $137 = $116 + 8 | 0;
     HEAP32[$137 >> 2] = $F4_0;
     var $138 = $116 + 12 | 0;
     HEAP32[$138 >> 2] = $121;
    }
    HEAP32[(__gm_ + 8 | 0) >> 2] = $102;
    HEAP32[(__gm_ + 20 | 0) >> 2] = $107;
    var $140 = $86;
    var $mem_0 = $140;
    __label__ = 331;
    break;
   }
   var $142 = HEAP32[(__gm_ + 4 | 0) >> 2];
   var $143 = ($142 | 0) == 0;
   if ($143) {
    var $nb_0 = $8;
    __label__ = 155;
    break;
   }
   var $145 = -$142 | 0;
   var $146 = $142 & $145;
   var $147 = $146 - 1 | 0;
   var $148 = $147 >>> 12;
   var $149 = $148 & 16;
   var $150 = $147 >>> ($149 >>> 0);
   var $151 = $150 >>> 5;
   var $152 = $151 & 8;
   var $153 = $150 >>> ($152 >>> 0);
   var $154 = $153 >>> 2;
   var $155 = $154 & 4;
   var $156 = $153 >>> ($155 >>> 0);
   var $157 = $156 >>> 1;
   var $158 = $157 & 2;
   var $159 = $156 >>> ($158 >>> 0);
   var $160 = $159 >>> 1;
   var $161 = $160 & 1;
   var $162 = $152 | $149;
   var $163 = $162 | $155;
   var $164 = $163 | $158;
   var $165 = $164 | $161;
   var $166 = $159 >>> ($161 >>> 0);
   var $167 = $165 + $166 | 0;
   var $168 = __gm_ + 304 + ($167 << 2) | 0;
   var $169 = HEAP32[$168 >> 2];
   var $170 = $169 + 4 | 0;
   var $171 = HEAP32[$170 >> 2];
   var $172 = $171 & -8;
   var $173 = $172 - $8 | 0;
   var $t_0_i = $169;
   var $v_0_i = $169;
   var $rsize_0_i = $173;
   while (1) {
    var $rsize_0_i;
    var $v_0_i;
    var $t_0_i;
    var $175 = $t_0_i + 16 | 0;
    var $176 = HEAP32[$175 >> 2];
    var $177 = ($176 | 0) == 0;
    if ($177) {
     var $179 = $t_0_i + 20 | 0;
     var $180 = HEAP32[$179 >> 2];
     var $181 = ($180 | 0) == 0;
     if ($181) {
      break;
     }
     var $182 = $180;
    } else {
     var $182 = $176;
    }
    var $182;
    var $183 = $182 + 4 | 0;
    var $184 = HEAP32[$183 >> 2];
    var $185 = $184 & -8;
    var $186 = $185 - $8 | 0;
    var $187 = $186 >>> 0 < $rsize_0_i >>> 0;
    var $_rsize_0_i = $187 ? $186 : $rsize_0_i;
    var $_v_0_i = $187 ? $182 : $v_0_i;
    var $t_0_i = $182;
    var $v_0_i = $_v_0_i;
    var $rsize_0_i = $_rsize_0_i;
   }
   var $189 = $v_0_i;
   var $190 = HEAP32[(__gm_ + 16 | 0) >> 2];
   var $191 = $189 >>> 0 < $190 >>> 0;
   do {
    if (!$191) {
     var $193 = $189 + $8 | 0;
     var $194 = $193;
     var $195 = $189 >>> 0 < $193 >>> 0;
     if (!$195) {
      break;
     }
     var $197 = $v_0_i + 24 | 0;
     var $198 = HEAP32[$197 >> 2];
     var $199 = $v_0_i + 12 | 0;
     var $200 = HEAP32[$199 >> 2];
     var $201 = ($200 | 0) == ($v_0_i | 0);
     do {
      if ($201) {
       var $212 = $v_0_i + 20 | 0;
       var $213 = HEAP32[$212 >> 2];
       var $214 = ($213 | 0) == 0;
       if ($214) {
        var $216 = $v_0_i + 16 | 0;
        var $217 = HEAP32[$216 >> 2];
        var $218 = ($217 | 0) == 0;
        if ($218) {
         var $R_1_i = 0;
         break;
        }
        var $RP_0_i = $216;
        var $R_0_i = $217;
       } else {
        var $RP_0_i = $212;
        var $R_0_i = $213;
        __label__ = 39;
       }
       while (1) {
        var $R_0_i;
        var $RP_0_i;
        var $219 = $R_0_i + 20 | 0;
        var $220 = HEAP32[$219 >> 2];
        var $221 = ($220 | 0) == 0;
        if (!$221) {
         var $RP_0_i = $219;
         var $R_0_i = $220;
         continue;
        }
        var $223 = $R_0_i + 16 | 0;
        var $224 = HEAP32[$223 >> 2];
        var $225 = ($224 | 0) == 0;
        if ($225) {
         break;
        }
        var $RP_0_i = $223;
        var $R_0_i = $224;
       }
       var $227 = $RP_0_i;
       var $228 = $227 >>> 0 < $190 >>> 0;
       if ($228) {
        _abort();
       } else {
        HEAP32[$RP_0_i >> 2] = 0;
        var $R_1_i = $R_0_i;
       }
      } else {
       var $203 = $v_0_i + 8 | 0;
       var $204 = HEAP32[$203 >> 2];
       var $205 = $204;
       var $206 = $205 >>> 0 < $190 >>> 0;
       if ($206) {
        _abort();
       } else {
        var $208 = $204 + 12 | 0;
        HEAP32[$208 >> 2] = $200;
        var $209 = $200 + 8 | 0;
        HEAP32[$209 >> 2] = $204;
        var $R_1_i = $200;
       }
      }
     } while (0);
     var $R_1_i;
     var $232 = ($198 | 0) == 0;
     $_$62 : do {
      if (!$232) {
       var $234 = $v_0_i + 28 | 0;
       var $235 = HEAP32[$234 >> 2];
       var $236 = __gm_ + 304 + ($235 << 2) | 0;
       var $237 = HEAP32[$236 >> 2];
       var $238 = ($v_0_i | 0) == ($237 | 0);
       do {
        if ($238) {
         HEAP32[$236 >> 2] = $R_1_i;
         var $cond_i = ($R_1_i | 0) == 0;
         if (!$cond_i) {
          break;
         }
         var $240 = HEAP32[$234 >> 2];
         var $241 = 1 << $240;
         var $242 = $241 ^ -1;
         var $243 = HEAP32[(__gm_ + 4 | 0) >> 2];
         var $244 = $243 & $242;
         HEAP32[(__gm_ + 4 | 0) >> 2] = $244;
         break $_$62;
        }
        var $246 = $198;
        var $247 = HEAP32[(__gm_ + 16 | 0) >> 2];
        var $248 = $246 >>> 0 < $247 >>> 0;
        if ($248) {
         _abort();
        } else {
         var $250 = $198 + 16 | 0;
         var $251 = HEAP32[$250 >> 2];
         var $252 = ($251 | 0) == ($v_0_i | 0);
         if ($252) {
          HEAP32[$250 >> 2] = $R_1_i;
         } else {
          var $255 = $198 + 20 | 0;
          HEAP32[$255 >> 2] = $R_1_i;
         }
         var $258 = ($R_1_i | 0) == 0;
         if ($258) {
          break $_$62;
         }
        }
       } while (0);
       var $260 = $R_1_i;
       var $261 = HEAP32[(__gm_ + 16 | 0) >> 2];
       var $262 = $260 >>> 0 < $261 >>> 0;
       if ($262) {
        _abort();
       } else {
        var $264 = $R_1_i + 24 | 0;
        HEAP32[$264 >> 2] = $198;
        var $265 = $v_0_i + 16 | 0;
        var $266 = HEAP32[$265 >> 2];
        var $267 = ($266 | 0) == 0;
        if (!$267) {
         var $269 = $266;
         var $270 = HEAP32[(__gm_ + 16 | 0) >> 2];
         var $271 = $269 >>> 0 < $270 >>> 0;
         if ($271) {
          _abort();
         } else {
          var $273 = $R_1_i + 16 | 0;
          HEAP32[$273 >> 2] = $266;
          var $274 = $266 + 24 | 0;
          HEAP32[$274 >> 2] = $R_1_i;
         }
        }
        var $277 = $v_0_i + 20 | 0;
        var $278 = HEAP32[$277 >> 2];
        var $279 = ($278 | 0) == 0;
        if ($279) {
         break;
        }
        var $281 = $278;
        var $282 = HEAP32[(__gm_ + 16 | 0) >> 2];
        var $283 = $281 >>> 0 < $282 >>> 0;
        if ($283) {
         _abort();
        } else {
         var $285 = $R_1_i + 20 | 0;
         HEAP32[$285 >> 2] = $278;
         var $286 = $278 + 24 | 0;
         HEAP32[$286 >> 2] = $R_1_i;
        }
       }
      }
     } while (0);
     var $290 = $rsize_0_i >>> 0 < 16;
     if ($290) {
      var $292 = $rsize_0_i + $8 | 0;
      var $293 = $292 | 3;
      var $294 = $v_0_i + 4 | 0;
      HEAP32[$294 >> 2] = $293;
      var $_sum4_i = $292 + 4 | 0;
      var $295 = $189 + $_sum4_i | 0;
      var $296 = $295;
      var $297 = HEAP32[$296 >> 2];
      var $298 = $297 | 1;
      HEAP32[$296 >> 2] = $298;
     } else {
      var $300 = $8 | 3;
      var $301 = $v_0_i + 4 | 0;
      HEAP32[$301 >> 2] = $300;
      var $302 = $rsize_0_i | 1;
      var $_sum_i33 = $8 | 4;
      var $303 = $189 + $_sum_i33 | 0;
      var $304 = $303;
      HEAP32[$304 >> 2] = $302;
      var $_sum1_i = $rsize_0_i + $8 | 0;
      var $305 = $189 + $_sum1_i | 0;
      var $306 = $305;
      HEAP32[$306 >> 2] = $rsize_0_i;
      var $307 = HEAP32[(__gm_ + 8 | 0) >> 2];
      var $308 = ($307 | 0) == 0;
      if (!$308) {
       var $310 = HEAP32[(__gm_ + 20 | 0) >> 2];
       var $311 = $307 >>> 3;
       var $312 = $307 >>> 2;
       var $313 = $312 & 1073741822;
       var $314 = __gm_ + 40 + ($313 << 2) | 0;
       var $315 = $314;
       var $316 = HEAP32[(__gm_ | 0) >> 2];
       var $317 = 1 << $311;
       var $318 = $316 & $317;
       var $319 = ($318 | 0) == 0;
       do {
        if ($319) {
         var $321 = $316 | $317;
         HEAP32[(__gm_ | 0) >> 2] = $321;
         var $_sum2_pre_i = $313 + 2 | 0;
         var $_pre_i = __gm_ + 40 + ($_sum2_pre_i << 2) | 0;
         var $F1_0_i = $315;
         var $_pre_phi_i = $_pre_i;
        } else {
         var $_sum3_i = $313 + 2 | 0;
         var $323 = __gm_ + 40 + ($_sum3_i << 2) | 0;
         var $324 = HEAP32[$323 >> 2];
         var $325 = $324;
         var $326 = HEAP32[(__gm_ + 16 | 0) >> 2];
         var $327 = $325 >>> 0 < $326 >>> 0;
         if (!$327) {
          var $F1_0_i = $324;
          var $_pre_phi_i = $323;
          break;
         }
         _abort();
        }
       } while (0);
       var $_pre_phi_i;
       var $F1_0_i;
       HEAP32[$_pre_phi_i >> 2] = $310;
       var $330 = $F1_0_i + 12 | 0;
       HEAP32[$330 >> 2] = $310;
       var $331 = $310 + 8 | 0;
       HEAP32[$331 >> 2] = $F1_0_i;
       var $332 = $310 + 12 | 0;
       HEAP32[$332 >> 2] = $315;
      }
      HEAP32[(__gm_ + 8 | 0) >> 2] = $rsize_0_i;
      HEAP32[(__gm_ + 20 | 0) >> 2] = $194;
     }
     var $335 = $v_0_i + 8 | 0;
     var $336 = $335;
     var $337 = ($335 | 0) == 0;
     if ($337) {
      var $nb_0 = $8;
      __label__ = 155;
      break $_$2;
     }
     var $mem_0 = $336;
     __label__ = 331;
     break $_$2;
    }
   } while (0);
   _abort();
  } else {
   var $339 = $bytes >>> 0 > 4294967231;
   if ($339) {
    var $nb_0 = -1;
    __label__ = 155;
    break;
   }
   var $341 = $bytes + 11 | 0;
   var $342 = $341 & -8;
   var $343 = HEAP32[(__gm_ + 4 | 0) >> 2];
   var $344 = ($343 | 0) == 0;
   if ($344) {
    var $nb_0 = $342;
    __label__ = 155;
    break;
   }
   var $346 = -$342 | 0;
   var $347 = $341 >>> 8;
   var $348 = ($347 | 0) == 0;
   do {
    if ($348) {
     var $idx_0_i = 0;
    } else {
     var $350 = $342 >>> 0 > 16777215;
     if ($350) {
      var $idx_0_i = 31;
      break;
     }
     var $352 = $347 + 1048320 | 0;
     var $353 = $352 >>> 16;
     var $354 = $353 & 8;
     var $355 = $347 << $354;
     var $356 = $355 + 520192 | 0;
     var $357 = $356 >>> 16;
     var $358 = $357 & 4;
     var $359 = $355 << $358;
     var $360 = $359 + 245760 | 0;
     var $361 = $360 >>> 16;
     var $362 = $361 & 2;
     var $363 = $358 | $354;
     var $364 = $363 | $362;
     var $365 = 14 - $364 | 0;
     var $366 = $359 << $362;
     var $367 = $366 >>> 15;
     var $368 = $365 + $367 | 0;
     var $369 = $368 << 1;
     var $370 = $368 + 7 | 0;
     var $371 = $342 >>> ($370 >>> 0);
     var $372 = $371 & 1;
     var $373 = $372 | $369;
     var $idx_0_i = $373;
    }
   } while (0);
   var $idx_0_i;
   var $375 = __gm_ + 304 + ($idx_0_i << 2) | 0;
   var $376 = HEAP32[$375 >> 2];
   var $377 = ($376 | 0) == 0;
   $_$110 : do {
    if ($377) {
     var $v_2_i = 0;
     var $rsize_2_i = $346;
     var $t_1_i = 0;
    } else {
     var $379 = ($idx_0_i | 0) == 31;
     if ($379) {
      var $384 = 0;
     } else {
      var $381 = $idx_0_i >>> 1;
      var $382 = 25 - $381 | 0;
      var $384 = $382;
     }
     var $384;
     var $385 = $342 << $384;
     var $v_0_i15 = 0;
     var $rsize_0_i14 = $346;
     var $t_0_i13 = $376;
     var $sizebits_0_i = $385;
     var $rst_0_i = 0;
     while (1) {
      var $rst_0_i;
      var $sizebits_0_i;
      var $t_0_i13;
      var $rsize_0_i14;
      var $v_0_i15;
      var $387 = $t_0_i13 + 4 | 0;
      var $388 = HEAP32[$387 >> 2];
      var $389 = $388 & -8;
      var $390 = $389 - $342 | 0;
      var $391 = $390 >>> 0 < $rsize_0_i14 >>> 0;
      if ($391) {
       var $393 = ($389 | 0) == ($342 | 0);
       if ($393) {
        var $v_2_i = $t_0_i13;
        var $rsize_2_i = $390;
        var $t_1_i = $t_0_i13;
        break $_$110;
       }
       var $v_1_i = $t_0_i13;
       var $rsize_1_i = $390;
      } else {
       var $v_1_i = $v_0_i15;
       var $rsize_1_i = $rsize_0_i14;
      }
      var $rsize_1_i;
      var $v_1_i;
      var $395 = $t_0_i13 + 20 | 0;
      var $396 = HEAP32[$395 >> 2];
      var $397 = $sizebits_0_i >>> 31;
      var $398 = $t_0_i13 + 16 + ($397 << 2) | 0;
      var $399 = HEAP32[$398 >> 2];
      var $400 = ($396 | 0) == 0;
      var $401 = ($396 | 0) == ($399 | 0);
      var $or_cond_i = $400 | $401;
      var $rst_1_i = $or_cond_i ? $rst_0_i : $396;
      var $402 = ($399 | 0) == 0;
      var $403 = $sizebits_0_i << 1;
      if ($402) {
       var $v_2_i = $v_1_i;
       var $rsize_2_i = $rsize_1_i;
       var $t_1_i = $rst_1_i;
       break $_$110;
      }
      var $v_0_i15 = $v_1_i;
      var $rsize_0_i14 = $rsize_1_i;
      var $t_0_i13 = $399;
      var $sizebits_0_i = $403;
      var $rst_0_i = $rst_1_i;
     }
    }
   } while (0);
   var $t_1_i;
   var $rsize_2_i;
   var $v_2_i;
   var $404 = ($t_1_i | 0) == 0;
   var $405 = ($v_2_i | 0) == 0;
   var $or_cond19_i = $404 & $405;
   if ($or_cond19_i) {
    var $407 = 2 << $idx_0_i;
    var $408 = -$407 | 0;
    var $409 = $407 | $408;
    var $410 = $343 & $409;
    var $411 = ($410 | 0) == 0;
    if ($411) {
     var $nb_0 = $342;
     __label__ = 155;
     break;
    }
    var $413 = -$410 | 0;
    var $414 = $410 & $413;
    var $415 = $414 - 1 | 0;
    var $416 = $415 >>> 12;
    var $417 = $416 & 16;
    var $418 = $415 >>> ($417 >>> 0);
    var $419 = $418 >>> 5;
    var $420 = $419 & 8;
    var $421 = $418 >>> ($420 >>> 0);
    var $422 = $421 >>> 2;
    var $423 = $422 & 4;
    var $424 = $421 >>> ($423 >>> 0);
    var $425 = $424 >>> 1;
    var $426 = $425 & 2;
    var $427 = $424 >>> ($426 >>> 0);
    var $428 = $427 >>> 1;
    var $429 = $428 & 1;
    var $430 = $420 | $417;
    var $431 = $430 | $423;
    var $432 = $431 | $426;
    var $433 = $432 | $429;
    var $434 = $427 >>> ($429 >>> 0);
    var $435 = $433 + $434 | 0;
    var $436 = __gm_ + 304 + ($435 << 2) | 0;
    var $437 = HEAP32[$436 >> 2];
    var $t_2_ph_i = $437;
   } else {
    var $t_2_ph_i = $t_1_i;
   }
   var $t_2_ph_i;
   var $438 = ($t_2_ph_i | 0) == 0;
   $_$125 : do {
    if ($438) {
     var $rsize_3_lcssa_i = $rsize_2_i;
     var $v_3_lcssa_i = $v_2_i;
    } else {
     var $t_224_i = $t_2_ph_i;
     var $rsize_325_i = $rsize_2_i;
     var $v_326_i = $v_2_i;
     while (1) {
      var $v_326_i;
      var $rsize_325_i;
      var $t_224_i;
      var $439 = $t_224_i + 4 | 0;
      var $440 = HEAP32[$439 >> 2];
      var $441 = $440 & -8;
      var $442 = $441 - $342 | 0;
      var $443 = $442 >>> 0 < $rsize_325_i >>> 0;
      var $_rsize_3_i = $443 ? $442 : $rsize_325_i;
      var $t_2_v_3_i = $443 ? $t_224_i : $v_326_i;
      var $444 = $t_224_i + 16 | 0;
      var $445 = HEAP32[$444 >> 2];
      var $446 = ($445 | 0) == 0;
      if (!$446) {
       var $t_224_i = $445;
       var $rsize_325_i = $_rsize_3_i;
       var $v_326_i = $t_2_v_3_i;
       continue;
      }
      var $447 = $t_224_i + 20 | 0;
      var $448 = HEAP32[$447 >> 2];
      var $449 = ($448 | 0) == 0;
      if ($449) {
       var $rsize_3_lcssa_i = $_rsize_3_i;
       var $v_3_lcssa_i = $t_2_v_3_i;
       break $_$125;
      }
      var $t_224_i = $448;
      var $rsize_325_i = $_rsize_3_i;
      var $v_326_i = $t_2_v_3_i;
     }
    }
   } while (0);
   var $v_3_lcssa_i;
   var $rsize_3_lcssa_i;
   var $450 = ($v_3_lcssa_i | 0) == 0;
   if ($450) {
    var $nb_0 = $342;
    __label__ = 155;
    break;
   }
   var $452 = HEAP32[(__gm_ + 8 | 0) >> 2];
   var $453 = $452 - $342 | 0;
   var $454 = $rsize_3_lcssa_i >>> 0 < $453 >>> 0;
   if (!$454) {
    var $nb_0 = $342;
    __label__ = 155;
    break;
   }
   var $456 = $v_3_lcssa_i;
   var $457 = HEAP32[(__gm_ + 16 | 0) >> 2];
   var $458 = $456 >>> 0 < $457 >>> 0;
   do {
    if (!$458) {
     var $460 = $456 + $342 | 0;
     var $461 = $460;
     var $462 = $456 >>> 0 < $460 >>> 0;
     if (!$462) {
      break;
     }
     var $464 = $v_3_lcssa_i + 24 | 0;
     var $465 = HEAP32[$464 >> 2];
     var $466 = $v_3_lcssa_i + 12 | 0;
     var $467 = HEAP32[$466 >> 2];
     var $468 = ($467 | 0) == ($v_3_lcssa_i | 0);
     do {
      if ($468) {
       var $479 = $v_3_lcssa_i + 20 | 0;
       var $480 = HEAP32[$479 >> 2];
       var $481 = ($480 | 0) == 0;
       if ($481) {
        var $483 = $v_3_lcssa_i + 16 | 0;
        var $484 = HEAP32[$483 >> 2];
        var $485 = ($484 | 0) == 0;
        if ($485) {
         var $R_1_i19 = 0;
         break;
        }
        var $RP_0_i17 = $483;
        var $R_0_i16 = $484;
       } else {
        var $RP_0_i17 = $479;
        var $R_0_i16 = $480;
        __label__ = 103;
       }
       while (1) {
        var $R_0_i16;
        var $RP_0_i17;
        var $486 = $R_0_i16 + 20 | 0;
        var $487 = HEAP32[$486 >> 2];
        var $488 = ($487 | 0) == 0;
        if (!$488) {
         var $RP_0_i17 = $486;
         var $R_0_i16 = $487;
         continue;
        }
        var $490 = $R_0_i16 + 16 | 0;
        var $491 = HEAP32[$490 >> 2];
        var $492 = ($491 | 0) == 0;
        if ($492) {
         break;
        }
        var $RP_0_i17 = $490;
        var $R_0_i16 = $491;
       }
       var $494 = $RP_0_i17;
       var $495 = $494 >>> 0 < $457 >>> 0;
       if ($495) {
        _abort();
       } else {
        HEAP32[$RP_0_i17 >> 2] = 0;
        var $R_1_i19 = $R_0_i16;
       }
      } else {
       var $470 = $v_3_lcssa_i + 8 | 0;
       var $471 = HEAP32[$470 >> 2];
       var $472 = $471;
       var $473 = $472 >>> 0 < $457 >>> 0;
       if ($473) {
        _abort();
       } else {
        var $475 = $471 + 12 | 0;
        HEAP32[$475 >> 2] = $467;
        var $476 = $467 + 8 | 0;
        HEAP32[$476 >> 2] = $471;
        var $R_1_i19 = $467;
       }
      }
     } while (0);
     var $R_1_i19;
     var $499 = ($465 | 0) == 0;
     $_$151 : do {
      if (!$499) {
       var $501 = $v_3_lcssa_i + 28 | 0;
       var $502 = HEAP32[$501 >> 2];
       var $503 = __gm_ + 304 + ($502 << 2) | 0;
       var $504 = HEAP32[$503 >> 2];
       var $505 = ($v_3_lcssa_i | 0) == ($504 | 0);
       do {
        if ($505) {
         HEAP32[$503 >> 2] = $R_1_i19;
         var $cond_i20 = ($R_1_i19 | 0) == 0;
         if (!$cond_i20) {
          break;
         }
         var $507 = HEAP32[$501 >> 2];
         var $508 = 1 << $507;
         var $509 = $508 ^ -1;
         var $510 = HEAP32[(__gm_ + 4 | 0) >> 2];
         var $511 = $510 & $509;
         HEAP32[(__gm_ + 4 | 0) >> 2] = $511;
         break $_$151;
        }
        var $513 = $465;
        var $514 = HEAP32[(__gm_ + 16 | 0) >> 2];
        var $515 = $513 >>> 0 < $514 >>> 0;
        if ($515) {
         _abort();
        } else {
         var $517 = $465 + 16 | 0;
         var $518 = HEAP32[$517 >> 2];
         var $519 = ($518 | 0) == ($v_3_lcssa_i | 0);
         if ($519) {
          HEAP32[$517 >> 2] = $R_1_i19;
         } else {
          var $522 = $465 + 20 | 0;
          HEAP32[$522 >> 2] = $R_1_i19;
         }
         var $525 = ($R_1_i19 | 0) == 0;
         if ($525) {
          break $_$151;
         }
        }
       } while (0);
       var $527 = $R_1_i19;
       var $528 = HEAP32[(__gm_ + 16 | 0) >> 2];
       var $529 = $527 >>> 0 < $528 >>> 0;
       if ($529) {
        _abort();
       } else {
        var $531 = $R_1_i19 + 24 | 0;
        HEAP32[$531 >> 2] = $465;
        var $532 = $v_3_lcssa_i + 16 | 0;
        var $533 = HEAP32[$532 >> 2];
        var $534 = ($533 | 0) == 0;
        if (!$534) {
         var $536 = $533;
         var $537 = HEAP32[(__gm_ + 16 | 0) >> 2];
         var $538 = $536 >>> 0 < $537 >>> 0;
         if ($538) {
          _abort();
         } else {
          var $540 = $R_1_i19 + 16 | 0;
          HEAP32[$540 >> 2] = $533;
          var $541 = $533 + 24 | 0;
          HEAP32[$541 >> 2] = $R_1_i19;
         }
        }
        var $544 = $v_3_lcssa_i + 20 | 0;
        var $545 = HEAP32[$544 >> 2];
        var $546 = ($545 | 0) == 0;
        if ($546) {
         break;
        }
        var $548 = $545;
        var $549 = HEAP32[(__gm_ + 16 | 0) >> 2];
        var $550 = $548 >>> 0 < $549 >>> 0;
        if ($550) {
         _abort();
        } else {
         var $552 = $R_1_i19 + 20 | 0;
         HEAP32[$552 >> 2] = $545;
         var $553 = $545 + 24 | 0;
         HEAP32[$553 >> 2] = $R_1_i19;
        }
       }
      }
     } while (0);
     var $557 = $rsize_3_lcssa_i >>> 0 < 16;
     $_$179 : do {
      if ($557) {
       var $559 = $rsize_3_lcssa_i + $342 | 0;
       var $560 = $559 | 3;
       var $561 = $v_3_lcssa_i + 4 | 0;
       HEAP32[$561 >> 2] = $560;
       var $_sum18_i = $559 + 4 | 0;
       var $562 = $456 + $_sum18_i | 0;
       var $563 = $562;
       var $564 = HEAP32[$563 >> 2];
       var $565 = $564 | 1;
       HEAP32[$563 >> 2] = $565;
      } else {
       var $567 = $342 | 3;
       var $568 = $v_3_lcssa_i + 4 | 0;
       HEAP32[$568 >> 2] = $567;
       var $569 = $rsize_3_lcssa_i | 1;
       var $_sum_i2232 = $342 | 4;
       var $570 = $456 + $_sum_i2232 | 0;
       var $571 = $570;
       HEAP32[$571 >> 2] = $569;
       var $_sum1_i23 = $rsize_3_lcssa_i + $342 | 0;
       var $572 = $456 + $_sum1_i23 | 0;
       var $573 = $572;
       HEAP32[$573 >> 2] = $rsize_3_lcssa_i;
       var $574 = $rsize_3_lcssa_i >>> 0 < 256;
       if ($574) {
        var $576 = $rsize_3_lcssa_i >>> 3;
        var $577 = $rsize_3_lcssa_i >>> 2;
        var $578 = $577 & 1073741822;
        var $579 = __gm_ + 40 + ($578 << 2) | 0;
        var $580 = $579;
        var $581 = HEAP32[(__gm_ | 0) >> 2];
        var $582 = 1 << $576;
        var $583 = $581 & $582;
        var $584 = ($583 | 0) == 0;
        do {
         if ($584) {
          var $586 = $581 | $582;
          HEAP32[(__gm_ | 0) >> 2] = $586;
          var $_sum14_pre_i = $578 + 2 | 0;
          var $_pre_i24 = __gm_ + 40 + ($_sum14_pre_i << 2) | 0;
          var $F5_0_i = $580;
          var $_pre_phi_i25 = $_pre_i24;
         } else {
          var $_sum17_i = $578 + 2 | 0;
          var $588 = __gm_ + 40 + ($_sum17_i << 2) | 0;
          var $589 = HEAP32[$588 >> 2];
          var $590 = $589;
          var $591 = HEAP32[(__gm_ + 16 | 0) >> 2];
          var $592 = $590 >>> 0 < $591 >>> 0;
          if (!$592) {
           var $F5_0_i = $589;
           var $_pre_phi_i25 = $588;
           break;
          }
          _abort();
         }
        } while (0);
        var $_pre_phi_i25;
        var $F5_0_i;
        HEAP32[$_pre_phi_i25 >> 2] = $461;
        var $595 = $F5_0_i + 12 | 0;
        HEAP32[$595 >> 2] = $461;
        var $_sum15_i = $342 + 8 | 0;
        var $596 = $456 + $_sum15_i | 0;
        var $597 = $596;
        HEAP32[$597 >> 2] = $F5_0_i;
        var $_sum16_i = $342 + 12 | 0;
        var $598 = $456 + $_sum16_i | 0;
        var $599 = $598;
        HEAP32[$599 >> 2] = $580;
       } else {
        var $601 = $460;
        var $602 = $rsize_3_lcssa_i >>> 8;
        var $603 = ($602 | 0) == 0;
        do {
         if ($603) {
          var $I7_0_i = 0;
         } else {
          var $605 = $rsize_3_lcssa_i >>> 0 > 16777215;
          if ($605) {
           var $I7_0_i = 31;
           break;
          }
          var $607 = $602 + 1048320 | 0;
          var $608 = $607 >>> 16;
          var $609 = $608 & 8;
          var $610 = $602 << $609;
          var $611 = $610 + 520192 | 0;
          var $612 = $611 >>> 16;
          var $613 = $612 & 4;
          var $614 = $610 << $613;
          var $615 = $614 + 245760 | 0;
          var $616 = $615 >>> 16;
          var $617 = $616 & 2;
          var $618 = $613 | $609;
          var $619 = $618 | $617;
          var $620 = 14 - $619 | 0;
          var $621 = $614 << $617;
          var $622 = $621 >>> 15;
          var $623 = $620 + $622 | 0;
          var $624 = $623 << 1;
          var $625 = $623 + 7 | 0;
          var $626 = $rsize_3_lcssa_i >>> ($625 >>> 0);
          var $627 = $626 & 1;
          var $628 = $627 | $624;
          var $I7_0_i = $628;
         }
        } while (0);
        var $I7_0_i;
        var $630 = __gm_ + 304 + ($I7_0_i << 2) | 0;
        var $_sum2_i = $342 + 28 | 0;
        var $631 = $456 + $_sum2_i | 0;
        var $632 = $631;
        HEAP32[$632 >> 2] = $I7_0_i;
        var $_sum3_i26 = $342 + 16 | 0;
        var $633 = $456 + $_sum3_i26 | 0;
        var $_sum4_i27 = $342 + 20 | 0;
        var $634 = $456 + $_sum4_i27 | 0;
        var $635 = $634;
        HEAP32[$635 >> 2] = 0;
        var $636 = $633;
        HEAP32[$636 >> 2] = 0;
        var $637 = HEAP32[(__gm_ + 4 | 0) >> 2];
        var $638 = 1 << $I7_0_i;
        var $639 = $637 & $638;
        var $640 = ($639 | 0) == 0;
        if ($640) {
         var $642 = $637 | $638;
         HEAP32[(__gm_ + 4 | 0) >> 2] = $642;
         HEAP32[$630 >> 2] = $601;
         var $643 = $630;
         var $_sum5_i = $342 + 24 | 0;
         var $644 = $456 + $_sum5_i | 0;
         var $645 = $644;
         HEAP32[$645 >> 2] = $643;
         var $_sum6_i = $342 + 12 | 0;
         var $646 = $456 + $_sum6_i | 0;
         var $647 = $646;
         HEAP32[$647 >> 2] = $601;
         var $_sum7_i = $342 + 8 | 0;
         var $648 = $456 + $_sum7_i | 0;
         var $649 = $648;
         HEAP32[$649 >> 2] = $601;
        } else {
         var $651 = HEAP32[$630 >> 2];
         var $652 = ($I7_0_i | 0) == 31;
         if ($652) {
          var $657 = 0;
         } else {
          var $654 = $I7_0_i >>> 1;
          var $655 = 25 - $654 | 0;
          var $657 = $655;
         }
         var $657;
         var $658 = $rsize_3_lcssa_i << $657;
         var $K12_0_i = $658;
         var $T_0_i = $651;
         while (1) {
          var $T_0_i;
          var $K12_0_i;
          var $660 = $T_0_i + 4 | 0;
          var $661 = HEAP32[$660 >> 2];
          var $662 = $661 & -8;
          var $663 = ($662 | 0) == ($rsize_3_lcssa_i | 0);
          if ($663) {
           var $683 = $T_0_i + 8 | 0;
           var $684 = HEAP32[$683 >> 2];
           var $685 = $T_0_i;
           var $686 = HEAP32[(__gm_ + 16 | 0) >> 2];
           var $687 = $685 >>> 0 < $686 >>> 0;
           do {
            if (!$687) {
             var $689 = $684;
             var $690 = $689 >>> 0 < $686 >>> 0;
             if ($690) {
              break;
             }
             var $692 = $684 + 12 | 0;
             HEAP32[$692 >> 2] = $601;
             HEAP32[$683 >> 2] = $601;
             var $_sum8_i = $342 + 8 | 0;
             var $693 = $456 + $_sum8_i | 0;
             var $694 = $693;
             HEAP32[$694 >> 2] = $684;
             var $_sum9_i = $342 + 12 | 0;
             var $695 = $456 + $_sum9_i | 0;
             var $696 = $695;
             HEAP32[$696 >> 2] = $T_0_i;
             var $_sum10_i = $342 + 24 | 0;
             var $697 = $456 + $_sum10_i | 0;
             var $698 = $697;
             HEAP32[$698 >> 2] = 0;
             break $_$179;
            }
           } while (0);
           _abort();
          } else {
           var $665 = $K12_0_i >>> 31;
           var $666 = $T_0_i + 16 + ($665 << 2) | 0;
           var $667 = HEAP32[$666 >> 2];
           var $668 = ($667 | 0) == 0;
           var $669 = $K12_0_i << 1;
           if (!$668) {
            var $K12_0_i = $669;
            var $T_0_i = $667;
            continue;
           }
           var $671 = $666;
           var $672 = HEAP32[(__gm_ + 16 | 0) >> 2];
           var $673 = $671 >>> 0 < $672 >>> 0;
           if (!$673) {
            HEAP32[$666 >> 2] = $601;
            var $_sum11_i = $342 + 24 | 0;
            var $675 = $456 + $_sum11_i | 0;
            var $676 = $675;
            HEAP32[$676 >> 2] = $T_0_i;
            var $_sum12_i = $342 + 12 | 0;
            var $677 = $456 + $_sum12_i | 0;
            var $678 = $677;
            HEAP32[$678 >> 2] = $601;
            var $_sum13_i = $342 + 8 | 0;
            var $679 = $456 + $_sum13_i | 0;
            var $680 = $679;
            HEAP32[$680 >> 2] = $601;
            break $_$179;
           }
           _abort();
          }
         }
        }
       }
      }
     } while (0);
     var $700 = $v_3_lcssa_i + 8 | 0;
     var $701 = $700;
     var $702 = ($700 | 0) == 0;
     if ($702) {
      var $nb_0 = $342;
      __label__ = 155;
      break $_$2;
     }
     var $mem_0 = $701;
     __label__ = 331;
     break $_$2;
    }
   } while (0);
   _abort();
  }
 } while (0);
 $_$215 : do {
  if (__label__ == 155) {
   var $nb_0;
   var $703 = HEAP32[(__gm_ + 8 | 0) >> 2];
   var $704 = $nb_0 >>> 0 > $703 >>> 0;
   if ($704) {
    var $732 = HEAP32[(__gm_ + 12 | 0) >> 2];
    var $733 = $nb_0 >>> 0 < $732 >>> 0;
    if ($733) {
     var $735 = $732 - $nb_0 | 0;
     HEAP32[(__gm_ + 12 | 0) >> 2] = $735;
     var $736 = HEAP32[(__gm_ + 24 | 0) >> 2];
     var $737 = $736;
     var $738 = $737 + $nb_0 | 0;
     var $739 = $738;
     HEAP32[(__gm_ + 24 | 0) >> 2] = $739;
     var $740 = $735 | 1;
     var $_sum = $nb_0 + 4 | 0;
     var $741 = $737 + $_sum | 0;
     var $742 = $741;
     HEAP32[$742 >> 2] = $740;
     var $743 = $nb_0 | 3;
     var $744 = $736 + 4 | 0;
     HEAP32[$744 >> 2] = $743;
     var $745 = $736 + 8 | 0;
     var $746 = $745;
     var $mem_0 = $746;
    } else {
     var $748 = HEAP32[(_mparams | 0) >> 2];
     var $749 = ($748 | 0) == 0;
     do {
      if ($749) {
       var $751 = HEAP32[(_mparams | 0) >> 2];
       var $752 = ($751 | 0) == 0;
       if (!$752) {
        break;
       }
       var $754 = _sysconf(8);
       var $755 = $754 - 1 | 0;
       var $756 = $755 & $754;
       var $757 = ($756 | 0) == 0;
       if ($757) {
        HEAP32[(_mparams + 8 | 0) >> 2] = $754;
        HEAP32[(_mparams + 4 | 0) >> 2] = $754;
        HEAP32[(_mparams + 12 | 0) >> 2] = -1;
        HEAP32[(_mparams + 16 | 0) >> 2] = 2097152;
        HEAP32[(_mparams + 20 | 0) >> 2] = 0;
        HEAP32[(__gm_ + 440 | 0) >> 2] = 0;
        var $760 = _time(0);
        var $761 = $760 & -16;
        var $762 = $761 ^ 1431655768;
        HEAP32[(_mparams | 0) >> 2] = $762;
       } else {
        _abort();
       }
      }
     } while (0);
     var $763 = HEAP32[(__gm_ + 440 | 0) >> 2];
     var $764 = $763 & 4;
     var $765 = ($764 | 0) == 0;
     $_$234 : do {
      if ($765) {
       var $767 = HEAP32[(__gm_ + 24 | 0) >> 2];
       var $768 = ($767 | 0) == 0;
       $_$236 : do {
        if (!$768) {
         var $770 = $767;
         var $sp_0_i_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_0_i_i;
          var $772 = $sp_0_i_i | 0;
          var $773 = HEAP32[$772 >> 2];
          var $774 = $773 >>> 0 > $770 >>> 0;
          if (!$774) {
           var $776 = $sp_0_i_i + 4 | 0;
           var $777 = HEAP32[$776 >> 2];
           var $778 = $773 + $777 | 0;
           var $779 = $778 >>> 0 > $770 >>> 0;
           if ($779) {
            break;
           }
          }
          var $781 = $sp_0_i_i + 8 | 0;
          var $782 = HEAP32[$781 >> 2];
          var $783 = ($782 | 0) == 0;
          if ($783) {
           __label__ = 174;
           break $_$236;
          }
          var $sp_0_i_i = $782;
         }
         var $784 = ($sp_0_i_i | 0) == 0;
         if ($784) {
          __label__ = 174;
          break;
         }
         var $810 = HEAP32[(__gm_ + 12 | 0) >> 2];
         var $811 = HEAP32[(_mparams + 8 | 0) >> 2];
         var $812 = $nb_0 + 47 | 0;
         var $813 = $812 - $810 | 0;
         var $814 = $813 + $811 | 0;
         var $815 = -$811 | 0;
         var $816 = $814 & $815;
         var $817 = $816 >>> 0 < 2147483647;
         if (!$817) {
          var $tsize_0242932_ph_i = 0;
          __label__ = 189;
          break;
         }
         var $819 = _sbrk($816);
         var $820 = HEAP32[$772 >> 2];
         var $821 = HEAP32[$776 >> 2];
         var $822 = $820 + $821 | 0;
         var $823 = ($819 | 0) == ($822 | 0);
         var $_1_i = $823 ? $816 : 0;
         var $_2_i = $823 ? $819 : -1;
         var $tbase_0_i = $_2_i;
         var $tsize_0_i = $_1_i;
         var $asize_1_i = $816;
         var $br_0_i = $819;
         __label__ = 181;
         break;
        }
        __label__ = 174;
       } while (0);
       do {
        if (__label__ == 174) {
         var $785 = _sbrk(0);
         var $786 = ($785 | 0) == -1;
         if ($786) {
          var $tsize_0242932_ph_i = 0;
          __label__ = 189;
          break;
         }
         var $788 = HEAP32[(_mparams + 8 | 0) >> 2];
         var $789 = $nb_0 + 47 | 0;
         var $790 = $789 + $788 | 0;
         var $791 = -$788 | 0;
         var $792 = $790 & $791;
         var $793 = $785;
         var $794 = HEAP32[(_mparams + 4 | 0) >> 2];
         var $795 = $794 - 1 | 0;
         var $796 = $795 & $793;
         var $797 = ($796 | 0) == 0;
         if ($797) {
          var $asize_0_i = $792;
         } else {
          var $799 = $795 + $793 | 0;
          var $800 = -$794 | 0;
          var $801 = $799 & $800;
          var $802 = $792 - $793 | 0;
          var $803 = $802 + $801 | 0;
          var $asize_0_i = $803;
         }
         var $asize_0_i;
         var $805 = $asize_0_i >>> 0 < 2147483647;
         if (!$805) {
          var $tsize_0242932_ph_i = 0;
          __label__ = 189;
          break;
         }
         var $807 = _sbrk($asize_0_i);
         var $808 = ($807 | 0) == ($785 | 0);
         var $asize_0__i = $808 ? $asize_0_i : 0;
         var $__i = $808 ? $785 : -1;
         var $tbase_0_i = $__i;
         var $tsize_0_i = $asize_0__i;
         var $asize_1_i = $asize_0_i;
         var $br_0_i = $807;
         __label__ = 181;
         break;
        }
       } while (0);
       $_$253 : do {
        if (__label__ == 181) {
         var $br_0_i;
         var $asize_1_i;
         var $tsize_0_i;
         var $tbase_0_i;
         var $825 = -$asize_1_i | 0;
         var $826 = ($tbase_0_i | 0) == -1;
         if (!$826) {
          var $tsize_242_i = $tsize_0_i;
          var $tbase_243_i = $tbase_0_i;
          __label__ = 194;
          break $_$234;
         }
         var $828 = ($br_0_i | 0) != -1;
         var $829 = $asize_1_i >>> 0 < 2147483647;
         var $or_cond_i28 = $828 & $829;
         do {
          if ($or_cond_i28) {
           var $831 = $nb_0 + 48 | 0;
           var $832 = $asize_1_i >>> 0 < $831 >>> 0;
           if (!$832) {
            var $asize_2_i = $asize_1_i;
            break;
           }
           var $834 = HEAP32[(_mparams + 8 | 0) >> 2];
           var $835 = $nb_0 + 47 | 0;
           var $836 = $835 - $asize_1_i | 0;
           var $837 = $836 + $834 | 0;
           var $838 = -$834 | 0;
           var $839 = $837 & $838;
           var $840 = $839 >>> 0 < 2147483647;
           if (!$840) {
            var $asize_2_i = $asize_1_i;
            break;
           }
           var $842 = _sbrk($839);
           var $843 = ($842 | 0) == -1;
           if ($843) {
            var $847 = _sbrk($825);
            var $tsize_0242932_ph_i = $tsize_0_i;
            break $_$253;
           }
           var $845 = $839 + $asize_1_i | 0;
           var $asize_2_i = $845;
          } else {
           var $asize_2_i = $asize_1_i;
          }
         } while (0);
         var $asize_2_i;
         var $849 = ($br_0_i | 0) == -1;
         if (!$849) {
          var $tsize_242_i = $asize_2_i;
          var $tbase_243_i = $br_0_i;
          __label__ = 194;
          break $_$234;
         }
         var $852 = HEAP32[(__gm_ + 440 | 0) >> 2];
         var $853 = $852 | 4;
         HEAP32[(__gm_ + 440 | 0) >> 2] = $853;
         var $tsize_137_i = $tsize_0_i;
         __label__ = 191;
         break $_$234;
        }
       } while (0);
       var $tsize_0242932_ph_i;
       var $850 = HEAP32[(__gm_ + 440 | 0) >> 2];
       var $851 = $850 | 4;
       HEAP32[(__gm_ + 440 | 0) >> 2] = $851;
       var $tsize_137_i = $tsize_0242932_ph_i;
       __label__ = 191;
       break;
      }
      var $tsize_137_i = 0;
      __label__ = 191;
     } while (0);
     do {
      if (__label__ == 191) {
       var $tsize_137_i;
       var $854 = HEAP32[(_mparams + 8 | 0) >> 2];
       var $855 = $nb_0 + 47 | 0;
       var $856 = $855 + $854 | 0;
       var $857 = -$854 | 0;
       var $858 = $856 & $857;
       var $859 = $858 >>> 0 < 2147483647;
       if (!$859) {
        __label__ = 330;
        break;
       }
       var $861 = _sbrk($858);
       var $862 = _sbrk(0);
       var $notlhs_i = ($861 | 0) != -1;
       var $notrhs_i = ($862 | 0) != -1;
       var $or_cond3_not_i = $notrhs_i & $notlhs_i;
       var $863 = $861 >>> 0 < $862 >>> 0;
       var $or_cond4_i = $or_cond3_not_i & $863;
       if (!$or_cond4_i) {
        __label__ = 330;
        break;
       }
       var $864 = $862;
       var $865 = $861;
       var $866 = $864 - $865 | 0;
       var $867 = $nb_0 + 40 | 0;
       var $868 = $866 >>> 0 > $867 >>> 0;
       var $_tsize_1_i = $868 ? $866 : $tsize_137_i;
       var $_tbase_1_i = $868 ? $861 : -1;
       var $869 = ($_tbase_1_i | 0) == -1;
       if ($869) {
        __label__ = 330;
        break;
       }
       var $tsize_242_i = $_tsize_1_i;
       var $tbase_243_i = $_tbase_1_i;
       __label__ = 194;
       break;
      }
     } while (0);
     do {
      if (__label__ == 194) {
       var $tbase_243_i;
       var $tsize_242_i;
       var $870 = HEAP32[(__gm_ + 432 | 0) >> 2];
       var $871 = $870 + $tsize_242_i | 0;
       HEAP32[(__gm_ + 432 | 0) >> 2] = $871;
       var $872 = HEAP32[(__gm_ + 436 | 0) >> 2];
       var $873 = $871 >>> 0 > $872 >>> 0;
       if ($873) {
        HEAP32[(__gm_ + 436 | 0) >> 2] = $871;
       }
       var $876 = HEAP32[(__gm_ + 24 | 0) >> 2];
       var $877 = ($876 | 0) == 0;
       $_$275 : do {
        if ($877) {
         var $879 = HEAP32[(__gm_ + 16 | 0) >> 2];
         var $880 = ($879 | 0) == 0;
         var $881 = $tbase_243_i >>> 0 < $879 >>> 0;
         var $or_cond5_i = $880 | $881;
         if ($or_cond5_i) {
          HEAP32[(__gm_ + 16 | 0) >> 2] = $tbase_243_i;
         }
         HEAP32[(__gm_ + 444 | 0) >> 2] = $tbase_243_i;
         HEAP32[(__gm_ + 448 | 0) >> 2] = $tsize_242_i;
         HEAP32[(__gm_ + 456 | 0) >> 2] = 0;
         var $884 = HEAP32[(_mparams | 0) >> 2];
         HEAP32[(__gm_ + 36 | 0) >> 2] = $884;
         HEAP32[(__gm_ + 32 | 0) >> 2] = -1;
         var $i_02_i_i = 0;
         while (1) {
          var $i_02_i_i;
          var $886 = $i_02_i_i << 1;
          var $887 = __gm_ + 40 + ($886 << 2) | 0;
          var $888 = $887;
          var $_sum_i_i = $886 + 3 | 0;
          var $889 = __gm_ + 40 + ($_sum_i_i << 2) | 0;
          HEAP32[$889 >> 2] = $888;
          var $_sum1_i_i = $886 + 2 | 0;
          var $890 = __gm_ + 40 + ($_sum1_i_i << 2) | 0;
          HEAP32[$890 >> 2] = $888;
          var $891 = $i_02_i_i + 1 | 0;
          var $exitcond_i_i = ($891 | 0) == 32;
          if ($exitcond_i_i) {
           break;
          }
          var $i_02_i_i = $891;
         }
         var $892 = $tbase_243_i + 8 | 0;
         var $893 = $892;
         var $894 = $893 & 7;
         var $895 = ($894 | 0) == 0;
         if ($895) {
          var $899 = 0;
         } else {
          var $897 = -$893 | 0;
          var $898 = $897 & 7;
          var $899 = $898;
         }
         var $899;
         var $900 = $tbase_243_i + $899 | 0;
         var $901 = $900;
         var $902 = $tsize_242_i - 40 | 0;
         var $903 = $902 - $899 | 0;
         HEAP32[(__gm_ + 24 | 0) >> 2] = $901;
         HEAP32[(__gm_ + 12 | 0) >> 2] = $903;
         var $904 = $903 | 1;
         var $_sum_i9_i = $899 + 4 | 0;
         var $905 = $tbase_243_i + $_sum_i9_i | 0;
         var $906 = $905;
         HEAP32[$906 >> 2] = $904;
         var $_sum2_i_i = $tsize_242_i - 36 | 0;
         var $907 = $tbase_243_i + $_sum2_i_i | 0;
         var $908 = $907;
         HEAP32[$908 >> 2] = 40;
         var $909 = HEAP32[(_mparams + 16 | 0) >> 2];
         HEAP32[(__gm_ + 28 | 0) >> 2] = $909;
        } else {
         var $sp_0_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_0_i;
          var $910 = ($sp_0_i | 0) == 0;
          if ($910) {
           break;
          }
          var $912 = $sp_0_i | 0;
          var $913 = HEAP32[$912 >> 2];
          var $914 = $sp_0_i + 4 | 0;
          var $915 = HEAP32[$914 >> 2];
          var $916 = $913 + $915 | 0;
          var $917 = ($tbase_243_i | 0) == ($916 | 0);
          if ($917) {
           var $921 = $sp_0_i + 12 | 0;
           var $922 = HEAP32[$921 >> 2];
           var $923 = $922 & 8;
           var $924 = ($923 | 0) == 0;
           if (!$924) {
            break;
           }
           var $926 = $876;
           var $927 = $926 >>> 0 >= $913 >>> 0;
           var $928 = $926 >>> 0 < $tbase_243_i >>> 0;
           var $or_cond44_i = $927 & $928;
           if (!$or_cond44_i) {
            break;
           }
           var $930 = $915 + $tsize_242_i | 0;
           HEAP32[$914 >> 2] = $930;
           var $931 = HEAP32[(__gm_ + 24 | 0) >> 2];
           var $932 = HEAP32[(__gm_ + 12 | 0) >> 2];
           var $933 = $932 + $tsize_242_i | 0;
           var $934 = $931;
           var $935 = $931 + 8 | 0;
           var $936 = $935;
           var $937 = $936 & 7;
           var $938 = ($937 | 0) == 0;
           if ($938) {
            var $942 = 0;
           } else {
            var $940 = -$936 | 0;
            var $941 = $940 & 7;
            var $942 = $941;
           }
           var $942;
           var $943 = $934 + $942 | 0;
           var $944 = $943;
           var $945 = $933 - $942 | 0;
           HEAP32[(__gm_ + 24 | 0) >> 2] = $944;
           HEAP32[(__gm_ + 12 | 0) >> 2] = $945;
           var $946 = $945 | 1;
           var $_sum_i13_i = $942 + 4 | 0;
           var $947 = $934 + $_sum_i13_i | 0;
           var $948 = $947;
           HEAP32[$948 >> 2] = $946;
           var $_sum2_i14_i = $933 + 4 | 0;
           var $949 = $934 + $_sum2_i14_i | 0;
           var $950 = $949;
           HEAP32[$950 >> 2] = 40;
           var $951 = HEAP32[(_mparams + 16 | 0) >> 2];
           HEAP32[(__gm_ + 28 | 0) >> 2] = $951;
           break $_$275;
          }
          var $919 = $sp_0_i + 8 | 0;
          var $920 = HEAP32[$919 >> 2];
          var $sp_0_i = $920;
         }
         var $952 = HEAP32[(__gm_ + 16 | 0) >> 2];
         var $953 = $tbase_243_i >>> 0 < $952 >>> 0;
         if ($953) {
          HEAP32[(__gm_ + 16 | 0) >> 2] = $tbase_243_i;
         }
         var $955 = $tbase_243_i + $tsize_242_i | 0;
         var $sp_1_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_1_i;
          var $957 = ($sp_1_i | 0) == 0;
          if ($957) {
           __label__ = 293;
           break;
          }
          var $959 = $sp_1_i | 0;
          var $960 = HEAP32[$959 >> 2];
          var $961 = ($960 | 0) == ($955 | 0);
          if ($961) {
           __label__ = 218;
           break;
          }
          var $963 = $sp_1_i + 8 | 0;
          var $964 = HEAP32[$963 >> 2];
          var $sp_1_i = $964;
         }
         do {
          if (__label__ == 218) {
           var $965 = $sp_1_i + 12 | 0;
           var $966 = HEAP32[$965 >> 2];
           var $967 = $966 & 8;
           var $968 = ($967 | 0) == 0;
           if (!$968) {
            break;
           }
           HEAP32[$959 >> 2] = $tbase_243_i;
           var $970 = $sp_1_i + 4 | 0;
           var $971 = HEAP32[$970 >> 2];
           var $972 = $971 + $tsize_242_i | 0;
           HEAP32[$970 >> 2] = $972;
           var $973 = $tbase_243_i + 8 | 0;
           var $974 = $973;
           var $975 = $974 & 7;
           var $976 = ($975 | 0) == 0;
           if ($976) {
            var $981 = 0;
           } else {
            var $978 = -$974 | 0;
            var $979 = $978 & 7;
            var $981 = $979;
           }
           var $981;
           var $982 = $tbase_243_i + $981 | 0;
           var $_sum79_i = $tsize_242_i + 8 | 0;
           var $983 = $tbase_243_i + $_sum79_i | 0;
           var $984 = $983;
           var $985 = $984 & 7;
           var $986 = ($985 | 0) == 0;
           if ($986) {
            var $991 = 0;
           } else {
            var $988 = -$984 | 0;
            var $989 = $988 & 7;
            var $991 = $989;
           }
           var $991;
           var $_sum80_i = $991 + $tsize_242_i | 0;
           var $992 = $tbase_243_i + $_sum80_i | 0;
           var $993 = $992;
           var $994 = $992;
           var $995 = $982;
           var $996 = $994 - $995 | 0;
           var $_sum_i16_i = $981 + $nb_0 | 0;
           var $997 = $tbase_243_i + $_sum_i16_i | 0;
           var $998 = $997;
           var $999 = $996 - $nb_0 | 0;
           var $1000 = $nb_0 | 3;
           var $_sum1_i17_i = $981 + 4 | 0;
           var $1001 = $tbase_243_i + $_sum1_i17_i | 0;
           var $1002 = $1001;
           HEAP32[$1002 >> 2] = $1000;
           var $1003 = HEAP32[(__gm_ + 24 | 0) >> 2];
           var $1004 = ($993 | 0) == ($1003 | 0);
           $_$314 : do {
            if ($1004) {
             var $1006 = HEAP32[(__gm_ + 12 | 0) >> 2];
             var $1007 = $1006 + $999 | 0;
             HEAP32[(__gm_ + 12 | 0) >> 2] = $1007;
             HEAP32[(__gm_ + 24 | 0) >> 2] = $998;
             var $1008 = $1007 | 1;
             var $_sum42_i_i = $_sum_i16_i + 4 | 0;
             var $1009 = $tbase_243_i + $_sum42_i_i | 0;
             var $1010 = $1009;
             HEAP32[$1010 >> 2] = $1008;
            } else {
             var $1012 = HEAP32[(__gm_ + 20 | 0) >> 2];
             var $1013 = ($993 | 0) == ($1012 | 0);
             if ($1013) {
              var $1015 = HEAP32[(__gm_ + 8 | 0) >> 2];
              var $1016 = $1015 + $999 | 0;
              HEAP32[(__gm_ + 8 | 0) >> 2] = $1016;
              HEAP32[(__gm_ + 20 | 0) >> 2] = $998;
              var $1017 = $1016 | 1;
              var $_sum40_i_i = $_sum_i16_i + 4 | 0;
              var $1018 = $tbase_243_i + $_sum40_i_i | 0;
              var $1019 = $1018;
              HEAP32[$1019 >> 2] = $1017;
              var $_sum41_i_i = $1016 + $_sum_i16_i | 0;
              var $1020 = $tbase_243_i + $_sum41_i_i | 0;
              var $1021 = $1020;
              HEAP32[$1021 >> 2] = $1016;
             } else {
              var $_sum2_i18_i = $tsize_242_i + 4 | 0;
              var $_sum81_i = $_sum2_i18_i + $991 | 0;
              var $1023 = $tbase_243_i + $_sum81_i | 0;
              var $1024 = $1023;
              var $1025 = HEAP32[$1024 >> 2];
              var $1026 = $1025 & 3;
              var $1027 = ($1026 | 0) == 1;
              if ($1027) {
               var $1029 = $1025 & -8;
               var $1030 = $1025 >>> 3;
               var $1031 = $1025 >>> 0 < 256;
               $_$322 : do {
                if ($1031) {
                 var $_sum3738_i_i = $991 | 8;
                 var $_sum91_i = $_sum3738_i_i + $tsize_242_i | 0;
                 var $1033 = $tbase_243_i + $_sum91_i | 0;
                 var $1034 = $1033;
                 var $1035 = HEAP32[$1034 >> 2];
                 var $_sum39_i_i = $tsize_242_i + 12 | 0;
                 var $_sum92_i = $_sum39_i_i + $991 | 0;
                 var $1036 = $tbase_243_i + $_sum92_i | 0;
                 var $1037 = $1036;
                 var $1038 = HEAP32[$1037 >> 2];
                 var $1039 = ($1035 | 0) == ($1038 | 0);
                 if ($1039) {
                  var $1041 = 1 << $1030;
                  var $1042 = $1041 ^ -1;
                  var $1043 = HEAP32[(__gm_ | 0) >> 2];
                  var $1044 = $1043 & $1042;
                  HEAP32[(__gm_ | 0) >> 2] = $1044;
                 } else {
                  var $1046 = $1025 >>> 2;
                  var $1047 = $1046 & 1073741822;
                  var $1048 = __gm_ + 40 + ($1047 << 2) | 0;
                  var $1049 = $1048;
                  var $1050 = ($1035 | 0) == ($1049 | 0);
                  do {
                   if ($1050) {
                    __label__ = 233;
                   } else {
                    var $1052 = $1035;
                    var $1053 = HEAP32[(__gm_ + 16 | 0) >> 2];
                    var $1054 = $1052 >>> 0 < $1053 >>> 0;
                    if ($1054) {
                     __label__ = 236;
                     break;
                    }
                    __label__ = 233;
                    break;
                   }
                  } while (0);
                  do {
                   if (__label__ == 233) {
                    var $1056 = ($1038 | 0) == ($1049 | 0);
                    if (!$1056) {
                     var $1058 = $1038;
                     var $1059 = HEAP32[(__gm_ + 16 | 0) >> 2];
                     var $1060 = $1058 >>> 0 < $1059 >>> 0;
                     if ($1060) {
                      break;
                     }
                    }
                    var $1061 = $1035 + 12 | 0;
                    HEAP32[$1061 >> 2] = $1038;
                    var $1062 = $1038 + 8 | 0;
                    HEAP32[$1062 >> 2] = $1035;
                    break $_$322;
                   }
                  } while (0);
                  _abort();
                 }
                } else {
                 var $1064 = $992;
                 var $_sum34_i_i = $991 | 24;
                 var $_sum82_i = $_sum34_i_i + $tsize_242_i | 0;
                 var $1065 = $tbase_243_i + $_sum82_i | 0;
                 var $1066 = $1065;
                 var $1067 = HEAP32[$1066 >> 2];
                 var $_sum5_i_i = $tsize_242_i + 12 | 0;
                 var $_sum83_i = $_sum5_i_i + $991 | 0;
                 var $1068 = $tbase_243_i + $_sum83_i | 0;
                 var $1069 = $1068;
                 var $1070 = HEAP32[$1069 >> 2];
                 var $1071 = ($1070 | 0) == ($1064 | 0);
                 do {
                  if ($1071) {
                   var $_sum67_i_i = $991 | 16;
                   var $_sum89_i = $_sum2_i18_i + $_sum67_i_i | 0;
                   var $1084 = $tbase_243_i + $_sum89_i | 0;
                   var $1085 = $1084;
                   var $1086 = HEAP32[$1085 >> 2];
                   var $1087 = ($1086 | 0) == 0;
                   if ($1087) {
                    var $_sum90_i = $_sum67_i_i + $tsize_242_i | 0;
                    var $1089 = $tbase_243_i + $_sum90_i | 0;
                    var $1090 = $1089;
                    var $1091 = HEAP32[$1090 >> 2];
                    var $1092 = ($1091 | 0) == 0;
                    if ($1092) {
                     var $R_1_i_i = 0;
                     break;
                    }
                    var $RP_0_i_i = $1090;
                    var $R_0_i_i = $1091;
                   } else {
                    var $RP_0_i_i = $1085;
                    var $R_0_i_i = $1086;
                    __label__ = 243;
                   }
                   while (1) {
                    var $R_0_i_i;
                    var $RP_0_i_i;
                    var $1093 = $R_0_i_i + 20 | 0;
                    var $1094 = HEAP32[$1093 >> 2];
                    var $1095 = ($1094 | 0) == 0;
                    if (!$1095) {
                     var $RP_0_i_i = $1093;
                     var $R_0_i_i = $1094;
                     continue;
                    }
                    var $1097 = $R_0_i_i + 16 | 0;
                    var $1098 = HEAP32[$1097 >> 2];
                    var $1099 = ($1098 | 0) == 0;
                    if ($1099) {
                     break;
                    }
                    var $RP_0_i_i = $1097;
                    var $R_0_i_i = $1098;
                   }
                   var $1101 = $RP_0_i_i;
                   var $1102 = HEAP32[(__gm_ + 16 | 0) >> 2];
                   var $1103 = $1101 >>> 0 < $1102 >>> 0;
                   if ($1103) {
                    _abort();
                   } else {
                    HEAP32[$RP_0_i_i >> 2] = 0;
                    var $R_1_i_i = $R_0_i_i;
                   }
                  } else {
                   var $_sum3536_i_i = $991 | 8;
                   var $_sum84_i = $_sum3536_i_i + $tsize_242_i | 0;
                   var $1073 = $tbase_243_i + $_sum84_i | 0;
                   var $1074 = $1073;
                   var $1075 = HEAP32[$1074 >> 2];
                   var $1076 = $1075;
                   var $1077 = HEAP32[(__gm_ + 16 | 0) >> 2];
                   var $1078 = $1076 >>> 0 < $1077 >>> 0;
                   if ($1078) {
                    _abort();
                   } else {
                    var $1080 = $1075 + 12 | 0;
                    HEAP32[$1080 >> 2] = $1070;
                    var $1081 = $1070 + 8 | 0;
                    HEAP32[$1081 >> 2] = $1075;
                    var $R_1_i_i = $1070;
                   }
                  }
                 } while (0);
                 var $R_1_i_i;
                 var $1107 = ($1067 | 0) == 0;
                 if ($1107) {
                  break;
                 }
                 var $_sum30_i_i = $tsize_242_i + 28 | 0;
                 var $_sum85_i = $_sum30_i_i + $991 | 0;
                 var $1109 = $tbase_243_i + $_sum85_i | 0;
                 var $1110 = $1109;
                 var $1111 = HEAP32[$1110 >> 2];
                 var $1112 = __gm_ + 304 + ($1111 << 2) | 0;
                 var $1113 = HEAP32[$1112 >> 2];
                 var $1114 = ($1064 | 0) == ($1113 | 0);
                 do {
                  if ($1114) {
                   HEAP32[$1112 >> 2] = $R_1_i_i;
                   var $cond_i_i = ($R_1_i_i | 0) == 0;
                   if (!$cond_i_i) {
                    break;
                   }
                   var $1116 = HEAP32[$1110 >> 2];
                   var $1117 = 1 << $1116;
                   var $1118 = $1117 ^ -1;
                   var $1119 = HEAP32[(__gm_ + 4 | 0) >> 2];
                   var $1120 = $1119 & $1118;
                   HEAP32[(__gm_ + 4 | 0) >> 2] = $1120;
                   break $_$322;
                  }
                  var $1122 = $1067;
                  var $1123 = HEAP32[(__gm_ + 16 | 0) >> 2];
                  var $1124 = $1122 >>> 0 < $1123 >>> 0;
                  if ($1124) {
                   _abort();
                  } else {
                   var $1126 = $1067 + 16 | 0;
                   var $1127 = HEAP32[$1126 >> 2];
                   var $1128 = ($1127 | 0) == ($1064 | 0);
                   if ($1128) {
                    HEAP32[$1126 >> 2] = $R_1_i_i;
                   } else {
                    var $1131 = $1067 + 20 | 0;
                    HEAP32[$1131 >> 2] = $R_1_i_i;
                   }
                   var $1134 = ($R_1_i_i | 0) == 0;
                   if ($1134) {
                    break $_$322;
                   }
                  }
                 } while (0);
                 var $1136 = $R_1_i_i;
                 var $1137 = HEAP32[(__gm_ + 16 | 0) >> 2];
                 var $1138 = $1136 >>> 0 < $1137 >>> 0;
                 if ($1138) {
                  _abort();
                 } else {
                  var $1140 = $R_1_i_i + 24 | 0;
                  HEAP32[$1140 >> 2] = $1067;
                  var $_sum3132_i_i = $991 | 16;
                  var $_sum86_i = $_sum3132_i_i + $tsize_242_i | 0;
                  var $1141 = $tbase_243_i + $_sum86_i | 0;
                  var $1142 = $1141;
                  var $1143 = HEAP32[$1142 >> 2];
                  var $1144 = ($1143 | 0) == 0;
                  if (!$1144) {
                   var $1146 = $1143;
                   var $1147 = HEAP32[(__gm_ + 16 | 0) >> 2];
                   var $1148 = $1146 >>> 0 < $1147 >>> 0;
                   if ($1148) {
                    _abort();
                   } else {
                    var $1150 = $R_1_i_i + 16 | 0;
                    HEAP32[$1150 >> 2] = $1143;
                    var $1151 = $1143 + 24 | 0;
                    HEAP32[$1151 >> 2] = $R_1_i_i;
                   }
                  }
                  var $_sum87_i = $_sum2_i18_i + $_sum3132_i_i | 0;
                  var $1154 = $tbase_243_i + $_sum87_i | 0;
                  var $1155 = $1154;
                  var $1156 = HEAP32[$1155 >> 2];
                  var $1157 = ($1156 | 0) == 0;
                  if ($1157) {
                   break;
                  }
                  var $1159 = $1156;
                  var $1160 = HEAP32[(__gm_ + 16 | 0) >> 2];
                  var $1161 = $1159 >>> 0 < $1160 >>> 0;
                  if ($1161) {
                   _abort();
                  } else {
                   var $1163 = $R_1_i_i + 20 | 0;
                   HEAP32[$1163 >> 2] = $1156;
                   var $1164 = $1156 + 24 | 0;
                   HEAP32[$1164 >> 2] = $R_1_i_i;
                  }
                 }
                }
               } while (0);
               var $_sum9_i_i = $1029 | $991;
               var $_sum88_i = $_sum9_i_i + $tsize_242_i | 0;
               var $1168 = $tbase_243_i + $_sum88_i | 0;
               var $1169 = $1168;
               var $1170 = $1029 + $999 | 0;
               var $oldfirst_0_i_i = $1169;
               var $qsize_0_i_i = $1170;
              } else {
               var $oldfirst_0_i_i = $993;
               var $qsize_0_i_i = $999;
              }
              var $qsize_0_i_i;
              var $oldfirst_0_i_i;
              var $1172 = $oldfirst_0_i_i + 4 | 0;
              var $1173 = HEAP32[$1172 >> 2];
              var $1174 = $1173 & -2;
              HEAP32[$1172 >> 2] = $1174;
              var $1175 = $qsize_0_i_i | 1;
              var $_sum10_i_i = $_sum_i16_i + 4 | 0;
              var $1176 = $tbase_243_i + $_sum10_i_i | 0;
              var $1177 = $1176;
              HEAP32[$1177 >> 2] = $1175;
              var $_sum11_i19_i = $qsize_0_i_i + $_sum_i16_i | 0;
              var $1178 = $tbase_243_i + $_sum11_i19_i | 0;
              var $1179 = $1178;
              HEAP32[$1179 >> 2] = $qsize_0_i_i;
              var $1180 = $qsize_0_i_i >>> 0 < 256;
              if ($1180) {
               var $1182 = $qsize_0_i_i >>> 3;
               var $1183 = $qsize_0_i_i >>> 2;
               var $1184 = $1183 & 1073741822;
               var $1185 = __gm_ + 40 + ($1184 << 2) | 0;
               var $1186 = $1185;
               var $1187 = HEAP32[(__gm_ | 0) >> 2];
               var $1188 = 1 << $1182;
               var $1189 = $1187 & $1188;
               var $1190 = ($1189 | 0) == 0;
               do {
                if ($1190) {
                 var $1192 = $1187 | $1188;
                 HEAP32[(__gm_ | 0) >> 2] = $1192;
                 var $_sum26_pre_i_i = $1184 + 2 | 0;
                 var $_pre_i_i = __gm_ + 40 + ($_sum26_pre_i_i << 2) | 0;
                 var $F4_0_i_i = $1186;
                 var $_pre_phi_i20_i = $_pre_i_i;
                } else {
                 var $_sum29_i_i = $1184 + 2 | 0;
                 var $1194 = __gm_ + 40 + ($_sum29_i_i << 2) | 0;
                 var $1195 = HEAP32[$1194 >> 2];
                 var $1196 = $1195;
                 var $1197 = HEAP32[(__gm_ + 16 | 0) >> 2];
                 var $1198 = $1196 >>> 0 < $1197 >>> 0;
                 if (!$1198) {
                  var $F4_0_i_i = $1195;
                  var $_pre_phi_i20_i = $1194;
                  break;
                 }
                 _abort();
                }
               } while (0);
               var $_pre_phi_i20_i;
               var $F4_0_i_i;
               HEAP32[$_pre_phi_i20_i >> 2] = $998;
               var $1201 = $F4_0_i_i + 12 | 0;
               HEAP32[$1201 >> 2] = $998;
               var $_sum27_i_i = $_sum_i16_i + 8 | 0;
               var $1202 = $tbase_243_i + $_sum27_i_i | 0;
               var $1203 = $1202;
               HEAP32[$1203 >> 2] = $F4_0_i_i;
               var $_sum28_i_i = $_sum_i16_i + 12 | 0;
               var $1204 = $tbase_243_i + $_sum28_i_i | 0;
               var $1205 = $1204;
               HEAP32[$1205 >> 2] = $1186;
              } else {
               var $1207 = $997;
               var $1208 = $qsize_0_i_i >>> 8;
               var $1209 = ($1208 | 0) == 0;
               do {
                if ($1209) {
                 var $I7_0_i_i = 0;
                } else {
                 var $1211 = $qsize_0_i_i >>> 0 > 16777215;
                 if ($1211) {
                  var $I7_0_i_i = 31;
                  break;
                 }
                 var $1213 = $1208 + 1048320 | 0;
                 var $1214 = $1213 >>> 16;
                 var $1215 = $1214 & 8;
                 var $1216 = $1208 << $1215;
                 var $1217 = $1216 + 520192 | 0;
                 var $1218 = $1217 >>> 16;
                 var $1219 = $1218 & 4;
                 var $1220 = $1216 << $1219;
                 var $1221 = $1220 + 245760 | 0;
                 var $1222 = $1221 >>> 16;
                 var $1223 = $1222 & 2;
                 var $1224 = $1219 | $1215;
                 var $1225 = $1224 | $1223;
                 var $1226 = 14 - $1225 | 0;
                 var $1227 = $1220 << $1223;
                 var $1228 = $1227 >>> 15;
                 var $1229 = $1226 + $1228 | 0;
                 var $1230 = $1229 << 1;
                 var $1231 = $1229 + 7 | 0;
                 var $1232 = $qsize_0_i_i >>> ($1231 >>> 0);
                 var $1233 = $1232 & 1;
                 var $1234 = $1233 | $1230;
                 var $I7_0_i_i = $1234;
                }
               } while (0);
               var $I7_0_i_i;
               var $1236 = __gm_ + 304 + ($I7_0_i_i << 2) | 0;
               var $_sum12_i_i = $_sum_i16_i + 28 | 0;
               var $1237 = $tbase_243_i + $_sum12_i_i | 0;
               var $1238 = $1237;
               HEAP32[$1238 >> 2] = $I7_0_i_i;
               var $_sum13_i_i = $_sum_i16_i + 16 | 0;
               var $1239 = $tbase_243_i + $_sum13_i_i | 0;
               var $_sum14_i_i = $_sum_i16_i + 20 | 0;
               var $1240 = $tbase_243_i + $_sum14_i_i | 0;
               var $1241 = $1240;
               HEAP32[$1241 >> 2] = 0;
               var $1242 = $1239;
               HEAP32[$1242 >> 2] = 0;
               var $1243 = HEAP32[(__gm_ + 4 | 0) >> 2];
               var $1244 = 1 << $I7_0_i_i;
               var $1245 = $1243 & $1244;
               var $1246 = ($1245 | 0) == 0;
               if ($1246) {
                var $1248 = $1243 | $1244;
                HEAP32[(__gm_ + 4 | 0) >> 2] = $1248;
                HEAP32[$1236 >> 2] = $1207;
                var $1249 = $1236;
                var $_sum15_i_i = $_sum_i16_i + 24 | 0;
                var $1250 = $tbase_243_i + $_sum15_i_i | 0;
                var $1251 = $1250;
                HEAP32[$1251 >> 2] = $1249;
                var $_sum16_i_i = $_sum_i16_i + 12 | 0;
                var $1252 = $tbase_243_i + $_sum16_i_i | 0;
                var $1253 = $1252;
                HEAP32[$1253 >> 2] = $1207;
                var $_sum17_i_i = $_sum_i16_i + 8 | 0;
                var $1254 = $tbase_243_i + $_sum17_i_i | 0;
                var $1255 = $1254;
                HEAP32[$1255 >> 2] = $1207;
               } else {
                var $1257 = HEAP32[$1236 >> 2];
                var $1258 = ($I7_0_i_i | 0) == 31;
                if ($1258) {
                 var $1263 = 0;
                } else {
                 var $1260 = $I7_0_i_i >>> 1;
                 var $1261 = 25 - $1260 | 0;
                 var $1263 = $1261;
                }
                var $1263;
                var $1264 = $qsize_0_i_i << $1263;
                var $K8_0_i_i = $1264;
                var $T_0_i21_i = $1257;
                while (1) {
                 var $T_0_i21_i;
                 var $K8_0_i_i;
                 var $1266 = $T_0_i21_i + 4 | 0;
                 var $1267 = HEAP32[$1266 >> 2];
                 var $1268 = $1267 & -8;
                 var $1269 = ($1268 | 0) == ($qsize_0_i_i | 0);
                 if ($1269) {
                  var $1289 = $T_0_i21_i + 8 | 0;
                  var $1290 = HEAP32[$1289 >> 2];
                  var $1291 = $T_0_i21_i;
                  var $1292 = HEAP32[(__gm_ + 16 | 0) >> 2];
                  var $1293 = $1291 >>> 0 < $1292 >>> 0;
                  do {
                   if (!$1293) {
                    var $1295 = $1290;
                    var $1296 = $1295 >>> 0 < $1292 >>> 0;
                    if ($1296) {
                     break;
                    }
                    var $1298 = $1290 + 12 | 0;
                    HEAP32[$1298 >> 2] = $1207;
                    HEAP32[$1289 >> 2] = $1207;
                    var $_sum20_i_i = $_sum_i16_i + 8 | 0;
                    var $1299 = $tbase_243_i + $_sum20_i_i | 0;
                    var $1300 = $1299;
                    HEAP32[$1300 >> 2] = $1290;
                    var $_sum21_i_i = $_sum_i16_i + 12 | 0;
                    var $1301 = $tbase_243_i + $_sum21_i_i | 0;
                    var $1302 = $1301;
                    HEAP32[$1302 >> 2] = $T_0_i21_i;
                    var $_sum22_i_i = $_sum_i16_i + 24 | 0;
                    var $1303 = $tbase_243_i + $_sum22_i_i | 0;
                    var $1304 = $1303;
                    HEAP32[$1304 >> 2] = 0;
                    break $_$314;
                   }
                  } while (0);
                  _abort();
                 } else {
                  var $1271 = $K8_0_i_i >>> 31;
                  var $1272 = $T_0_i21_i + 16 + ($1271 << 2) | 0;
                  var $1273 = HEAP32[$1272 >> 2];
                  var $1274 = ($1273 | 0) == 0;
                  var $1275 = $K8_0_i_i << 1;
                  if (!$1274) {
                   var $K8_0_i_i = $1275;
                   var $T_0_i21_i = $1273;
                   continue;
                  }
                  var $1277 = $1272;
                  var $1278 = HEAP32[(__gm_ + 16 | 0) >> 2];
                  var $1279 = $1277 >>> 0 < $1278 >>> 0;
                  if (!$1279) {
                   HEAP32[$1272 >> 2] = $1207;
                   var $_sum23_i_i = $_sum_i16_i + 24 | 0;
                   var $1281 = $tbase_243_i + $_sum23_i_i | 0;
                   var $1282 = $1281;
                   HEAP32[$1282 >> 2] = $T_0_i21_i;
                   var $_sum24_i_i = $_sum_i16_i + 12 | 0;
                   var $1283 = $tbase_243_i + $_sum24_i_i | 0;
                   var $1284 = $1283;
                   HEAP32[$1284 >> 2] = $1207;
                   var $_sum25_i_i = $_sum_i16_i + 8 | 0;
                   var $1285 = $tbase_243_i + $_sum25_i_i | 0;
                   var $1286 = $1285;
                   HEAP32[$1286 >> 2] = $1207;
                   break $_$314;
                  }
                  _abort();
                 }
                }
               }
              }
             }
            }
           } while (0);
           var $_sum1819_i_i = $981 | 8;
           var $1305 = $tbase_243_i + $_sum1819_i_i | 0;
           var $mem_0 = $1305;
           break $_$215;
          }
         } while (0);
         var $1306 = $876;
         var $sp_0_i_i_i = __gm_ + 444 | 0;
         while (1) {
          var $sp_0_i_i_i;
          var $1308 = $sp_0_i_i_i | 0;
          var $1309 = HEAP32[$1308 >> 2];
          var $1310 = $1309 >>> 0 > $1306 >>> 0;
          if (!$1310) {
           var $1312 = $sp_0_i_i_i + 4 | 0;
           var $1313 = HEAP32[$1312 >> 2];
           var $1314 = $1309 + $1313 | 0;
           var $1315 = $1314 >>> 0 > $1306 >>> 0;
           if ($1315) {
            var $1321 = $1309;
            var $1320 = $1313;
            break;
           }
          }
          var $1317 = $sp_0_i_i_i + 8 | 0;
          var $1318 = HEAP32[$1317 >> 2];
          var $1319 = ($1318 | 0) == 0;
          if (!$1319) {
           var $sp_0_i_i_i = $1318;
           continue;
          }
          var $_pre14_i_i = 4;
          var $1321 = 0;
          var $1320 = $_pre14_i_i;
          break;
         }
         var $1320;
         var $1321;
         var $1322 = $1321 + $1320 | 0;
         var $_sum1_i10_i = $1320 - 39 | 0;
         var $1323 = $1321 + $_sum1_i10_i | 0;
         var $1324 = $1323;
         var $1325 = $1324 & 7;
         var $1326 = ($1325 | 0) == 0;
         if ($1326) {
          var $1331 = 0;
         } else {
          var $1328 = -$1324 | 0;
          var $1329 = $1328 & 7;
          var $1331 = $1329;
         }
         var $1331;
         var $_sum_i11_i = $1320 - 47 | 0;
         var $_sum2_i12_i = $_sum_i11_i + $1331 | 0;
         var $1332 = $1321 + $_sum2_i12_i | 0;
         var $1333 = $876 + 16 | 0;
         var $1334 = $1333;
         var $1335 = $1332 >>> 0 < $1334 >>> 0;
         var $1336 = $1335 ? $1306 : $1332;
         var $1337 = $1336 + 8 | 0;
         var $1338 = $1337;
         var $1339 = $tbase_243_i + 8 | 0;
         var $1340 = $1339;
         var $1341 = $1340 & 7;
         var $1342 = ($1341 | 0) == 0;
         if ($1342) {
          var $1346 = 0;
         } else {
          var $1344 = -$1340 | 0;
          var $1345 = $1344 & 7;
          var $1346 = $1345;
         }
         var $1346;
         var $1347 = $tbase_243_i + $1346 | 0;
         var $1348 = $1347;
         var $1349 = $tsize_242_i - 40 | 0;
         var $1350 = $1349 - $1346 | 0;
         HEAP32[(__gm_ + 24 | 0) >> 2] = $1348;
         HEAP32[(__gm_ + 12 | 0) >> 2] = $1350;
         var $1351 = $1350 | 1;
         var $_sum_i_i_i = $1346 + 4 | 0;
         var $1352 = $tbase_243_i + $_sum_i_i_i | 0;
         var $1353 = $1352;
         HEAP32[$1353 >> 2] = $1351;
         var $_sum2_i_i_i = $tsize_242_i - 36 | 0;
         var $1354 = $tbase_243_i + $_sum2_i_i_i | 0;
         var $1355 = $1354;
         HEAP32[$1355 >> 2] = 40;
         var $1356 = HEAP32[(_mparams + 16 | 0) >> 2];
         HEAP32[(__gm_ + 28 | 0) >> 2] = $1356;
         var $1357 = $1336 + 4 | 0;
         var $1358 = $1357;
         HEAP32[$1358 >> 2] = 27;
         HEAP32[$1337 >> 2] = HEAP32[(__gm_ + 444 | 0) >> 2];
         HEAP32[$1337 + 4 >> 2] = HEAP32[(__gm_ + 444 | 0) + 4 >> 2];
         HEAP32[$1337 + 8 >> 2] = HEAP32[(__gm_ + 444 | 0) + 8 >> 2];
         HEAP32[$1337 + 12 >> 2] = HEAP32[(__gm_ + 444 | 0) + 12 >> 2];
         HEAP32[(__gm_ + 444 | 0) >> 2] = $tbase_243_i;
         HEAP32[(__gm_ + 448 | 0) >> 2] = $tsize_242_i;
         HEAP32[(__gm_ + 456 | 0) >> 2] = 0;
         HEAP32[(__gm_ + 452 | 0) >> 2] = $1338;
         var $1359 = $1336 + 28 | 0;
         var $1360 = $1359;
         HEAP32[$1360 >> 2] = 7;
         var $1361 = $1336 + 32 | 0;
         var $1362 = $1361 >>> 0 < $1322 >>> 0;
         $_$426 : do {
          if ($1362) {
           var $1363 = $1360;
           while (1) {
            var $1363;
            var $1364 = $1363 + 4 | 0;
            HEAP32[$1364 >> 2] = 7;
            var $1365 = $1363 + 8 | 0;
            var $1366 = $1365;
            var $1367 = $1366 >>> 0 < $1322 >>> 0;
            if (!$1367) {
             break $_$426;
            }
            var $1363 = $1364;
           }
          }
         } while (0);
         var $1368 = ($1336 | 0) == ($1306 | 0);
         if ($1368) {
          break;
         }
         var $1370 = $1336;
         var $1371 = $876;
         var $1372 = $1370 - $1371 | 0;
         var $1373 = $1306 + $1372 | 0;
         var $_sum3_i_i = $1372 + 4 | 0;
         var $1374 = $1306 + $_sum3_i_i | 0;
         var $1375 = $1374;
         var $1376 = HEAP32[$1375 >> 2];
         var $1377 = $1376 & -2;
         HEAP32[$1375 >> 2] = $1377;
         var $1378 = $1372 | 1;
         var $1379 = $876 + 4 | 0;
         HEAP32[$1379 >> 2] = $1378;
         var $1380 = $1373;
         HEAP32[$1380 >> 2] = $1372;
         var $1381 = $1372 >>> 0 < 256;
         if ($1381) {
          var $1383 = $1372 >>> 3;
          var $1384 = $1372 >>> 2;
          var $1385 = $1384 & 1073741822;
          var $1386 = __gm_ + 40 + ($1385 << 2) | 0;
          var $1387 = $1386;
          var $1388 = HEAP32[(__gm_ | 0) >> 2];
          var $1389 = 1 << $1383;
          var $1390 = $1388 & $1389;
          var $1391 = ($1390 | 0) == 0;
          do {
           if ($1391) {
            var $1393 = $1388 | $1389;
            HEAP32[(__gm_ | 0) >> 2] = $1393;
            var $_sum10_pre_i_i = $1385 + 2 | 0;
            var $_pre15_i_i = __gm_ + 40 + ($_sum10_pre_i_i << 2) | 0;
            var $F_0_i_i = $1387;
            var $_pre_phi_i_i = $_pre15_i_i;
           } else {
            var $_sum11_i_i = $1385 + 2 | 0;
            var $1395 = __gm_ + 40 + ($_sum11_i_i << 2) | 0;
            var $1396 = HEAP32[$1395 >> 2];
            var $1397 = $1396;
            var $1398 = HEAP32[(__gm_ + 16 | 0) >> 2];
            var $1399 = $1397 >>> 0 < $1398 >>> 0;
            if (!$1399) {
             var $F_0_i_i = $1396;
             var $_pre_phi_i_i = $1395;
             break;
            }
            _abort();
           }
          } while (0);
          var $_pre_phi_i_i;
          var $F_0_i_i;
          HEAP32[$_pre_phi_i_i >> 2] = $876;
          var $1402 = $F_0_i_i + 12 | 0;
          HEAP32[$1402 >> 2] = $876;
          var $1403 = $876 + 8 | 0;
          HEAP32[$1403 >> 2] = $F_0_i_i;
          var $1404 = $876 + 12 | 0;
          HEAP32[$1404 >> 2] = $1387;
         } else {
          var $1406 = $876;
          var $1407 = $1372 >>> 8;
          var $1408 = ($1407 | 0) == 0;
          do {
           if ($1408) {
            var $I1_0_i_i = 0;
           } else {
            var $1410 = $1372 >>> 0 > 16777215;
            if ($1410) {
             var $I1_0_i_i = 31;
             break;
            }
            var $1412 = $1407 + 1048320 | 0;
            var $1413 = $1412 >>> 16;
            var $1414 = $1413 & 8;
            var $1415 = $1407 << $1414;
            var $1416 = $1415 + 520192 | 0;
            var $1417 = $1416 >>> 16;
            var $1418 = $1417 & 4;
            var $1419 = $1415 << $1418;
            var $1420 = $1419 + 245760 | 0;
            var $1421 = $1420 >>> 16;
            var $1422 = $1421 & 2;
            var $1423 = $1418 | $1414;
            var $1424 = $1423 | $1422;
            var $1425 = 14 - $1424 | 0;
            var $1426 = $1419 << $1422;
            var $1427 = $1426 >>> 15;
            var $1428 = $1425 + $1427 | 0;
            var $1429 = $1428 << 1;
            var $1430 = $1428 + 7 | 0;
            var $1431 = $1372 >>> ($1430 >>> 0);
            var $1432 = $1431 & 1;
            var $1433 = $1432 | $1429;
            var $I1_0_i_i = $1433;
           }
          } while (0);
          var $I1_0_i_i;
          var $1435 = __gm_ + 304 + ($I1_0_i_i << 2) | 0;
          var $1436 = $876 + 28 | 0;
          var $I1_0_c_i_i = $I1_0_i_i;
          HEAP32[$1436 >> 2] = $I1_0_c_i_i;
          var $1437 = $876 + 20 | 0;
          HEAP32[$1437 >> 2] = 0;
          var $1438 = $876 + 16 | 0;
          HEAP32[$1438 >> 2] = 0;
          var $1439 = HEAP32[(__gm_ + 4 | 0) >> 2];
          var $1440 = 1 << $I1_0_i_i;
          var $1441 = $1439 & $1440;
          var $1442 = ($1441 | 0) == 0;
          if ($1442) {
           var $1444 = $1439 | $1440;
           HEAP32[(__gm_ + 4 | 0) >> 2] = $1444;
           HEAP32[$1435 >> 2] = $1406;
           var $1445 = $876 + 24 | 0;
           var $_c_i_i = $1435;
           HEAP32[$1445 >> 2] = $_c_i_i;
           var $1446 = $876 + 12 | 0;
           HEAP32[$1446 >> 2] = $876;
           var $1447 = $876 + 8 | 0;
           HEAP32[$1447 >> 2] = $876;
          } else {
           var $1449 = HEAP32[$1435 >> 2];
           var $1450 = ($I1_0_i_i | 0) == 31;
           if ($1450) {
            var $1455 = 0;
           } else {
            var $1452 = $I1_0_i_i >>> 1;
            var $1453 = 25 - $1452 | 0;
            var $1455 = $1453;
           }
           var $1455;
           var $1456 = $1372 << $1455;
           var $K2_0_i_i = $1456;
           var $T_0_i_i = $1449;
           while (1) {
            var $T_0_i_i;
            var $K2_0_i_i;
            var $1458 = $T_0_i_i + 4 | 0;
            var $1459 = HEAP32[$1458 >> 2];
            var $1460 = $1459 & -8;
            var $1461 = ($1460 | 0) == ($1372 | 0);
            if ($1461) {
             var $1478 = $T_0_i_i + 8 | 0;
             var $1479 = HEAP32[$1478 >> 2];
             var $1480 = $T_0_i_i;
             var $1481 = HEAP32[(__gm_ + 16 | 0) >> 2];
             var $1482 = $1480 >>> 0 < $1481 >>> 0;
             do {
              if (!$1482) {
               var $1484 = $1479;
               var $1485 = $1484 >>> 0 < $1481 >>> 0;
               if ($1485) {
                break;
               }
               var $1487 = $1479 + 12 | 0;
               HEAP32[$1487 >> 2] = $1406;
               HEAP32[$1478 >> 2] = $1406;
               var $1488 = $876 + 8 | 0;
               var $_c6_i_i = $1479;
               HEAP32[$1488 >> 2] = $_c6_i_i;
               var $1489 = $876 + 12 | 0;
               var $T_0_c_i_i = $T_0_i_i;
               HEAP32[$1489 >> 2] = $T_0_c_i_i;
               var $1490 = $876 + 24 | 0;
               HEAP32[$1490 >> 2] = 0;
               break $_$275;
              }
             } while (0);
             _abort();
            } else {
             var $1463 = $K2_0_i_i >>> 31;
             var $1464 = $T_0_i_i + 16 + ($1463 << 2) | 0;
             var $1465 = HEAP32[$1464 >> 2];
             var $1466 = ($1465 | 0) == 0;
             var $1467 = $K2_0_i_i << 1;
             if (!$1466) {
              var $K2_0_i_i = $1467;
              var $T_0_i_i = $1465;
              continue;
             }
             var $1469 = $1464;
             var $1470 = HEAP32[(__gm_ + 16 | 0) >> 2];
             var $1471 = $1469 >>> 0 < $1470 >>> 0;
             if (!$1471) {
              HEAP32[$1464 >> 2] = $1406;
              var $1473 = $876 + 24 | 0;
              var $T_0_c7_i_i = $T_0_i_i;
              HEAP32[$1473 >> 2] = $T_0_c7_i_i;
              var $1474 = $876 + 12 | 0;
              HEAP32[$1474 >> 2] = $876;
              var $1475 = $876 + 8 | 0;
              HEAP32[$1475 >> 2] = $876;
              break $_$275;
             }
             _abort();
            }
           }
          }
         }
        }
       } while (0);
       var $1491 = HEAP32[(__gm_ + 12 | 0) >> 2];
       var $1492 = $1491 >>> 0 > $nb_0 >>> 0;
       if (!$1492) {
        break;
       }
       var $1494 = $1491 - $nb_0 | 0;
       HEAP32[(__gm_ + 12 | 0) >> 2] = $1494;
       var $1495 = HEAP32[(__gm_ + 24 | 0) >> 2];
       var $1496 = $1495;
       var $1497 = $1496 + $nb_0 | 0;
       var $1498 = $1497;
       HEAP32[(__gm_ + 24 | 0) >> 2] = $1498;
       var $1499 = $1494 | 1;
       var $_sum_i30 = $nb_0 + 4 | 0;
       var $1500 = $1496 + $_sum_i30 | 0;
       var $1501 = $1500;
       HEAP32[$1501 >> 2] = $1499;
       var $1502 = $nb_0 | 3;
       var $1503 = $1495 + 4 | 0;
       HEAP32[$1503 >> 2] = $1502;
       var $1504 = $1495 + 8 | 0;
       var $1505 = $1504;
       var $mem_0 = $1505;
       break $_$215;
      }
     } while (0);
     var $1506 = ___errno();
     HEAP32[$1506 >> 2] = 12;
     var $mem_0 = 0;
    }
   } else {
    var $706 = $703 - $nb_0 | 0;
    var $707 = HEAP32[(__gm_ + 20 | 0) >> 2];
    var $708 = $706 >>> 0 > 15;
    if ($708) {
     var $710 = $707;
     var $711 = $710 + $nb_0 | 0;
     var $712 = $711;
     HEAP32[(__gm_ + 20 | 0) >> 2] = $712;
     HEAP32[(__gm_ + 8 | 0) >> 2] = $706;
     var $713 = $706 | 1;
     var $_sum2 = $nb_0 + 4 | 0;
     var $714 = $710 + $_sum2 | 0;
     var $715 = $714;
     HEAP32[$715 >> 2] = $713;
     var $716 = $710 + $703 | 0;
     var $717 = $716;
     HEAP32[$717 >> 2] = $706;
     var $718 = $nb_0 | 3;
     var $719 = $707 + 4 | 0;
     HEAP32[$719 >> 2] = $718;
    } else {
     HEAP32[(__gm_ + 8 | 0) >> 2] = 0;
     HEAP32[(__gm_ + 20 | 0) >> 2] = 0;
     var $721 = $703 | 3;
     var $722 = $707 + 4 | 0;
     HEAP32[$722 >> 2] = $721;
     var $723 = $707;
     var $_sum1 = $703 + 4 | 0;
     var $724 = $723 + $_sum1 | 0;
     var $725 = $724;
     var $726 = HEAP32[$725 >> 2];
     var $727 = $726 | 1;
     HEAP32[$725 >> 2] = $727;
    }
    var $729 = $707 + 8 | 0;
    var $730 = $729;
    var $mem_0 = $730;
   }
  }
 } while (0);
 var $mem_0;
 return $mem_0;
 return null;
}
function asm(x, y) { // asm-style code, without special asm requested so will not be fully optimized
 x = +x;
 y = y|0;
 var a = 0, b = +0, c = 0;
 var label = 0;
 a = cheez((y+~~x)|0)|0;
 b = a*a;
 fleefl(b|0, a|0);
}
function phi() {
 if (wat()) {
  var $10 = 1;
 } else {
  var $7=_init_mparams();
  var $8=(($7)|0)!=0;
  var $10 = $8;
 }
 var $10;
}
function intoCond() {
 var $115 = 22;
 var $499 = __ZN4llvm15BitstreamCursor4ReadEj($117, 32);
 var $NumWords = $499;
 var $500 = $115;
 var $501 = ($500 | 0) != 0;
 if ($501) {
  var $503 = $NumWords;
  var $504 = $115;
  HEAP32[$504 >> 2] = $503;
 }
}
function math(a, b, c, d) {
 var x, y, z, w;
 x = a;
 y = Math_abs(b);
 z = Math_fround(c);
 w = Math_imul(d);
 print(x + y + z + w);
}
function td(x, y) { // tempDoublePtr should invalidate each other
  HEAP32[tempDoublePtr>>2] = x;
  var xf = HEAPF32[tempDoublePtr>>2];
  HEAP32[tempDoublePtr>>2] = y;
  var yf = HEAPF32[tempDoublePtr>>2];
  func(xf, yf);
  //
  HEAPF64[tempDoublePtr>>3] = x;
  var xl = HEAP32[tempDoublePtr>>2];
  var xh = HEAP32[tempDoublePtr>>2];
  HEAPF64[tempDoublePtr>>3] = y;
  var yl = HEAP32[tempDoublePtr>>2];
  var yh = HEAP32[tempDoublePtr>>2];
  func(xl, xh, yl, yh);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b", "c", "f", "g", "h", "py", "r", "t", "f2", "f3", "llvm3_1", "_inflate", "_malloc", "_mallocNoU", "asm", "phi", "intoCond", "math", "td"]

