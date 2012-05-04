function f() {
  HEAP[123] = (GLOB[1] + 1) / 2;
}
function g(a1, a2) {
  var a = 1;
  var c = a * 2 - 1;
  a = c;
  foo(c);
  foo(2);
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
  if ($0 < 0) {
    __label__ = 1;
  } else {
    __label__ = 2;
  }
  var sadijn = new asd;
  sadijn2 = "qwe%sert";
  this.Module || (this.Module = {});
  var obj = {
    "quoted": 1,
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
  if (1) {
    otherGlob = glob;
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
  var $8 = HEAP[HEAP[__PyThreadState_Current] + 12] + 1 + 12;
  HEAP[$8] = 99;
}
function otherPy() {
  var $4 = HEAP[__PyThreadState_Current];
  var $5 = $4 + 12;
  var $7 = HEAP[$5] + 1;
  var $8 = $4 + 12;
  HEAP[$8] = $7;
}
var anon = (function(x) {
  var $4 = HEAP[__PyThreadState_Current];
  var $5 = $4 + 12;
  var $7 = HEAP[$5] + 1;
  var $8 = $4 + 12;
  HEAP[$8] = $7;
});
function r($0) {
  HEAP[$0 + 7] = 107;
}
function t() {
  if ($10 < $11) {
    __label__ = 3;
  }
  if (!($12 < $13)) {
    __label__ = 4;
  }
}
function f2() {
  var $_pre = HEAPU32[($vla + ($storemerge312 << 2) | 0) >> 2];
  var $storemerge312 = $storemerge312 + 1 | 0;
  var $8 = $_pre;
  c($8);
}
function f3($s, $tree, $k) {
  var $0 = HEAPU32[($s + 2908 + ($k << 2) | 0) >> 2];
  while (1) {
    HEAP32[($s + 2908 + ($storemerge_in << 2) | 0) >> 2] = $9;
  }
  HEAP32[($s + 2908 + ($storemerge_in << 2) | 0) >> 2] = $0;
}
function llvm3_1() {
  while (check()) {
    if ($curri_01 % $zj_0 == 0) {
      break;
    }
    var $j_0 = $aj_0 + 1;
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
      var $0 = HEAPU32[($strm + 28 | 0) >> 2];
      if (($0 | 0) == 0) {
        var $retval_0 = -2;
        break;
      }
      var $next_out = $strm + 12 | 0;
      var $1 = HEAP32[$next_out >> 2];
      if (($1 | 0) == 0) {
        var $retval_0 = -2;
        break;
      }
      var $next_in = $strm | 0;
      var $2 = HEAP32[$next_in >> 2];
      if (($2 | 0) == 0) {
        if (!((HEAP32[($strm + 4 | 0) >> 2] | 0) == 0)) {
          var $retval_0 = -2;
          break;
        }
      }
      var $4 = $0;
      var $mode = $0 | 0;
      var $5 = HEAP32[$mode >> 2];
      if (($5 | 0) == 11) {
        HEAP32[$mode >> 2] = 12;
        var $8 = HEAP32[$next_out >> 2];
        var $7 = HEAP32[$next_in >> 2];
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
      var $13 = $0 + 60 | 0;
      var $15 = $0 + 8 | 0;
      var $16 = $0 + 24 | 0;
      var $arrayidx = $hbuf | 0;
      var $arrayidx40 = $hbuf + 1 | 0;
      var $17 = $0 + 16 | 0;
      var $18 = $0 + 32 | 0;
      var $msg = $strm + 24 | 0;
      var $19 = $0 + 36 | 0;
      var $20 = $0 + 20 | 0;
      var $adler = $strm + 48 | 0;
      var $21 = $0 + 64 | 0;
      var $22 = $0 + 12 | 0;
      var $23 = ($flush - 5 | 0) >>> 0 < 2;
      var $24 = $0 + 4 | 0;
      var $cmp660 = ($flush | 0) == 6;
      var $25 = $0 + 7108 | 0;
      var $26 = $0 + 84 | 0;
      var $27 = $0 + 76 | 0;
      var $28 = $0 + 72 | 0;
      var $29 = $0 + 7112 | 0;
      var $30 = $0 + 68 | 0;
      var $31 = $0 + 44 | 0;
      var $32 = $0 + 7104 | 0;
      var $33 = $0 + 48 | 0;
      var $34 = $0 + 52 | 0;
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
      var $next861 = $0 + 108 | 0;
      var $42 = $next861;
      var $43 = $next861 | 0;
      var $arraydecay860_c = $0 + 1328 | 0;
      var $44 = $0 + 76 | 0;
      var $arraydecay864 = $lens;
      var $arraydecay867 = $0 + 752 | 0;
      var $45 = $0 + 624 | 0;
      var $46 = $0 + 80 | 0;
      var $47 = $0 + 88 | 0;
      var $48 = $0 + 80 | 0;
      var $ret_0 = 0;
      var $next_0 = $7;
      var $put_0 = $8;
      var $have_0 = $10;
      var $left_0 = $9;
      var $hold_0 = HEAP32[$11 >> 2];
      var $bits_0 = HEAP32[$13 >> 2];
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
            if (($50 | 0) == 0) {
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
              if (!($bits_1 >>> 0 < 16)) {
                break;
              }
              if (($have_1 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_1;
                var $have_58 = 0;
                var $hold_54 = $hold_1;
                var $bits_54 = $bits_1;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add = ((HEAPU8[$next_1] & 255) << $bits_1) + $hold_1 | 0;
              var $next_1 = $next_1 + 1 | 0;
              var $have_1 = $have_1 - 1 | 0;
              var $hold_1 = $add;
              var $bits_1 = $bits_1 + 8 | 0;
            }
            if (($50 & 2 | 0) != 0 & ($hold_1 | 0) == 35615) {
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
            if (($53 | 0) == 0) {
              var $54 = $50;
            } else {
              var $done = $53 + 48 | 0;
              HEAP32[$done >> 2] = -1;
              var $54 = HEAP32[$15 >> 2];
            }
            var $54;
            var $tobool56 = ($54 & 1 | 0) == 0;
            do {
              if (!$tobool56) {
                if (!((((($hold_1 << 8 & 65280) + ($hold_1 >>> 8) | 0) >>> 0) % 31 | 0) == 0)) {
                  break;
                }
                if (($hold_1 & 15 | 0) == 8) {
                  var $shr74 = $hold_1 >>> 4;
                  var $sub = $bits_1 - 4 | 0;
                  var $add77 = ($shr74 & 15) + 8 | 0;
                  var $55 = HEAPU32[$19 >> 2];
                  var $cmp78 = ($55 | 0) == 0;
                  do {
                    if (!$cmp78) {
                      if (!($add77 >>> 0 > $55 >>> 0)) {
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
                  HEAP32[$20 >> 2] = 1 << $add77;
                  var $call91 = _adler32(0, 0, 0);
                  HEAP32[$16 >> 2] = $call91;
                  HEAP32[$adler >> 2] = $call91;
                  HEAP32[$mode >> 2] = $hold_1 >>> 12 & 2 ^ 11;
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
              if (!($bits_2 >>> 0 < 16)) {
                break;
              }
              if (($have_2 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_2;
                var $have_58 = 0;
                var $hold_54 = $hold_2;
                var $bits_54 = $bits_2;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add113 = ((HEAPU8[$next_2] & 255) << $bits_2) + $hold_2 | 0;
              var $next_2 = $next_2 + 1 | 0;
              var $have_2 = $have_2 - 1 | 0;
              var $hold_2 = $add113;
              var $bits_2 = $bits_2 + 8 | 0;
            }
            HEAP32[$17 >> 2] = $hold_2;
            if (!(($hold_2 & 255 | 0) == 8)) {
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
            if (($hold_2 & 57344 | 0) == 0) {
              var $59 = HEAPU32[$18 >> 2];
              if (($59 | 0) == 0) {
                var $60 = $hold_2;
              } else {
                HEAP32[($59 | 0) >> 2] = $hold_2 >>> 8 & 1;
                var $60 = HEAP32[$17 >> 2];
              }
              var $60;
              if (!(($60 & 512 | 0) == 0)) {
                HEAP8[$arrayidx] = $hold_2 & 255;
                HEAP8[$arrayidx40] = $hold_2 >>> 8 & 255;
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
            var $next_11 = $next_0;
            var $have_11 = $have_0;
            var $hold_9 = $hold_0;
            var $bits_9 = $bits_0;
            var $89 = HEAP32[$17 >> 2];
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
              if (!($bits_14 >>> 0 < 32)) {
                break;
              }
              if (($have_18 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_18;
                var $have_58 = 0;
                var $hold_54 = $hold_14;
                var $bits_54 = $bits_14;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add564 = ((HEAPU8[$next_18] & 255) << $bits_14) + $hold_14 | 0;
              var $next_18 = $next_18 + 1 | 0;
              var $have_18 = $have_18 - 1 | 0;
              var $hold_14 = $add564;
              var $bits_14 = $bits_14 + 8 | 0;
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
            var $next_23 = $next_0;
            var $have_23 = $have_0;
            var $hold_19 = $hold_0 >>> ($and681 >>> 0);
            var $bits_19 = $bits_0 - $and681 | 0;
            while (1) {
              var $bits_19;
              var $hold_19;
              var $have_23;
              var $next_23;
              if (!($bits_19 >>> 0 < 32)) {
                break;
              }
              if (($have_23 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_23;
                var $have_58 = 0;
                var $hold_54 = $hold_19;
                var $bits_54 = $bits_19;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add701 = ((HEAPU8[$next_23] & 255) << $bits_19) + $hold_19 | 0;
              var $next_23 = $next_23 + 1 | 0;
              var $have_23 = $have_23 - 1 | 0;
              var $hold_19 = $add701;
              var $bits_19 = $bits_19 + 8 | 0;
            }
            var $and708 = $hold_19 & 65535;
            if (!(($and708 | 0) == ($hold_19 >>> 16 ^ 65535 | 0))) {
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
              if (!($bits_22 >>> 0 < 14)) {
                break;
              }
              if (($have_26 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_26;
                var $have_58 = 0;
                var $hold_54 = $hold_22;
                var $bits_54 = $bits_22;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add767 = ((HEAPU8[$next_26] & 255) << $bits_22) + $hold_22 | 0;
              var $next_26 = $next_26 + 1 | 0;
              var $have_26 = $have_26 - 1 | 0;
              var $hold_22 = $add767;
              var $bits_22 = $bits_22 + 8 | 0;
            }
            var $add775 = ($hold_22 & 31) + 257 | 0;
            HEAP32[$37 >> 2] = $add775;
            var $add782 = ($hold_22 >>> 5 & 31) + 1 | 0;
            HEAP32[$38 >> 2] = $add782;
            HEAP32[$39 >> 2] = ($hold_22 >>> 10 & 15) + 4 | 0;
            var $shr791 = $hold_22 >>> 14;
            var $sub792 = $bits_22 - 14 | 0;
            if ($add775 >>> 0 > 286 | $add782 >>> 0 > 30) {
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
            var $ret_4 = $ret_0;
            var $next_42 = $next_0;
            var $have_42 = $have_0;
            var $hold_38 = $hold_0;
            var $bits_38 = $bits_0;
            var $156 = HEAP32[$28 >> 2];
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
            var $ret_6 = $ret_0;
            var $next_48 = $next_0;
            var $have_48 = $have_0;
            var $hold_44 = $hold_0;
            var $bits_44 = $bits_0;
            var $167 = HEAP32[$28 >> 2];
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
            if (($left_0 | 0) == 0) {
              var $ret_8 = $ret_0;
              var $next_58 = $next_0;
              var $have_58 = $have_0;
              var $hold_54 = $hold_0;
              var $bits_54 = $bits_0;
              var $out_4 = $out_0;
              break $_$12;
            }
            HEAP8[$put_0] = HEAP32[$21 >> 2] & 255;
            HEAP32[$mode >> 2] = 20;
            var $ret_0_be = $ret_0;
            var $next_0_be = $next_0;
            var $put_0_be = $put_0 + 1 | 0;
            var $have_0_be = $have_0;
            var $left_0_be = $left_0 - 1 | 0;
            var $hold_0_be = $hold_0;
            var $bits_0_be = $bits_0;
            var $out_0_be = $out_0;
            __label__ = 268;
            break;
          } else if (($49 | 0) == 26) {
            var $tobool1626 = (HEAP32[$15 >> 2] | 0) == 0;
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
                  if (!($bits_48 >>> 0 < 32)) {
                    break;
                  }
                  if (($have_52 | 0) == 0) {
                    var $ret_8 = $ret_0;
                    var $next_58 = $next_52;
                    var $have_58 = 0;
                    var $hold_54 = $hold_48;
                    var $bits_54 = $bits_48;
                    var $out_4 = $out_0;
                    break $_$12;
                  }
                  var $add1642 = ((HEAPU8[$next_52] & 255) << $bits_48) + $hold_48 | 0;
                  var $next_52 = $next_52 + 1 | 0;
                  var $have_52 = $have_52 - 1 | 0;
                  var $hold_48 = $add1642;
                  var $bits_48 = $bits_48 + 8 | 0;
                }
                var $sub1649 = $out_0 - $left_0 | 0;
                var $add1650 = HEAP32[$total_out >> 2] + $sub1649 | 0;
                HEAP32[$total_out >> 2] = $add1650;
                var $add1651 = HEAP32[$36 >> 2] + $sub1649 | 0;
                HEAP32[$36 >> 2] = $add1651;
                if (!(($out_0 | 0) == ($left_0 | 0))) {
                  var $192 = HEAP32[$16 >> 2];
                  var $add_ptr1659 = $put_0 + (-$sub1649 | 0) | 0;
                  if ((HEAP32[$17 >> 2] | 0) == 0) {
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
                if ((HEAP32[$17 >> 2] | 0) == 0) {
                  var $add1685 = _llvm_bswap_i32($hold_48);
                  var $cond1687 = $add1685;
                } else {
                  var $cond1687 = $hold_48;
                }
                var $cond1687;
                if (($cond1687 | 0) == (HEAP32[$16 >> 2] | 0)) {
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
              if (!($bits_3 >>> 0 < 32)) {
                break;
              }
              if (($have_3 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_3;
                var $have_58 = 0;
                var $hold_54 = $hold_3;
                var $bits_54 = $bits_3;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add176 = ((HEAPU8[$next_3] & 255) << $bits_3) + $hold_3 | 0;
              var $next_3 = $next_3 + 1 | 0;
              var $have_3 = $have_3 - 1 | 0;
              var $hold_3 = $add176;
              var $bits_3 = $bits_3 + 8 | 0;
            }
            var $63 = HEAP32[$18 >> 2];
            if (!(($63 | 0) == 0)) {
              var $time = $63 + 4 | 0;
              HEAP32[$time >> 2] = $hold_3;
            }
            if (!((HEAP32[$17 >> 2] & 512 | 0) == 0)) {
              HEAP8[$arrayidx] = $hold_3 & 255;
              HEAP8[$arrayidx40] = $hold_3 >>> 8 & 255;
              HEAP8[$arrayidx199] = $hold_3 >>> 16 & 255;
              HEAP8[$arrayidx202] = $hold_3 >>> 24 & 255;
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
            if ((HEAP32[$22 >> 2] | 0) == 0) {
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
              if (!($122 >>> 0 < HEAPU32[$39 >> 2] >>> 0)) {
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
                if (!($bits_24 >>> 0 < 3)) {
                  break;
                }
                if (($have_28 | 0) == 0) {
                  var $ret_8 = $ret_0;
                  var $next_58 = $next_28;
                  var $have_58 = 0;
                  var $hold_54 = $hold_24;
                  var $bits_54 = $bits_24;
                  var $out_4 = $out_0;
                  break $_$12;
                }
                var $add829 = ((HEAPU8[$next_28] & 255) << $bits_24) + $hold_24 | 0;
                var $next_28 = $next_28 + 1 | 0;
                var $have_28 = $have_28 - 1 | 0;
                var $hold_24 = $add829;
                var $bits_24 = $bits_24 + 8 | 0;
              }
              HEAP32[$40 >> 2] = $122 + 1 | 0;
              HEAP16[($41 + ((HEAPU16[(_inflate_order + ($122 << 1) | 0) >> 1] & 65535) << 1) | 0) >> 1] = $hold_24 & 65535 & 7;
              var $next_27 = $next_28;
              var $have_27 = $have_28;
              var $hold_23 = $hold_24 >>> 3;
              var $bits_23 = $bits_24 - 3 | 0;
            }
            var $cmp850111 = $122 >>> 0 < 19;
            $_$131 : do {
              if ($cmp850111) {
                var $126 = $122;
                while (1) {
                  var $126;
                  HEAP32[$40 >> 2] = $126 + 1 | 0;
                  HEAP16[($41 + ((HEAPU16[(_inflate_order + ($126 << 1) | 0) >> 1] & 65535) << 1) | 0) >> 1] = 0;
                  var $_pr = HEAPU32[$40 >> 2];
                  if (!($_pr >>> 0 < 19)) {
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
            if (($call868 | 0) == 0) {
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
            var $tobool1702 = (HEAP32[$15 >> 2] | 0) == 0;
            do {
              if (!$tobool1702) {
                if ((HEAP32[$17 >> 2] | 0) == 0) {
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
                  if (!($bits_51 >>> 0 < 32)) {
                    break;
                  }
                  if (($have_55 | 0) == 0) {
                    var $ret_8 = $ret_0;
                    var $next_58 = $next_55;
                    var $have_58 = 0;
                    var $hold_54 = $hold_51;
                    var $bits_54 = $bits_51;
                    var $out_4 = $out_2;
                    break $_$12;
                  }
                  var $add1721 = ((HEAPU8[$next_55] & 255) << $bits_51) + $hold_51 | 0;
                  var $next_55 = $next_55 + 1 | 0;
                  var $have_55 = $have_55 - 1 | 0;
                  var $hold_51 = $add1721;
                  var $bits_51 = $bits_51 + 8 | 0;
                }
                if (($hold_51 | 0) == (HEAP32[$36 >> 2] | 0)) {
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
              if (!($bits_4 >>> 0 < 16)) {
                break;
              }
              if (($have_4 | 0) == 0) {
                var $ret_8 = $ret_0;
                var $next_58 = $next_4;
                var $have_58 = 0;
                var $hold_54 = $hold_4;
                var $bits_54 = $bits_4;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add227 = ((HEAPU8[$next_4] & 255) << $bits_4) + $hold_4 | 0;
              var $next_4 = $next_4 + 1 | 0;
              var $have_4 = $have_4 - 1 | 0;
              var $hold_4 = $add227;
              var $bits_4 = $bits_4 + 8 | 0;
            }
            var $67 = HEAP32[$18 >> 2];
            if (!(($67 | 0) == 0)) {
              var $xflags = $67 + 8 | 0;
              HEAP32[$xflags >> 2] = $hold_4 & 255;
              var $os = HEAP32[$18 >> 2] + 12 | 0;
              HEAP32[$os >> 2] = $hold_4 >>> 8;
            }
            if (!((HEAP32[$17 >> 2] & 512 | 0) == 0)) {
              HEAP8[$arrayidx] = $hold_4 & 255;
              HEAP8[$arrayidx40] = $hold_4 >>> 8 & 255;
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
            if (($119 | 0) == 0) {
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
            var $copy_3 = $119 >>> 0 > $have_25 >>> 0 ? $have_25 : $119;
            var $copy_4 = $copy_3 >>> 0 > $left_0 >>> 0 ? $left_0 : $copy_3;
            if (($copy_4 | 0) == 0) {
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
            var $sub749 = HEAP32[$21 >> 2] - $copy_4 | 0;
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
              var $add881 = HEAP32[$38 >> 2] + $129 | 0;
              if ($128 >>> 0 < $add881 >>> 0) {
                var $sub888 = (1 << HEAP32[$26 >> 2]) - 1 | 0;
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
                  var $conv893 = HEAPU8[$132 + ($and889 << 2) + 1 | 0] & 255;
                  if (!($conv893 >>> 0 > $bits_26 >>> 0)) {
                    break;
                  }
                  if (($have_30 | 0) == 0) {
                    var $ret_8 = $ret_1_ph;
                    var $next_58 = $next_30;
                    var $have_58 = 0;
                    var $hold_54 = $hold_26;
                    var $bits_54 = $bits_26;
                    var $out_4 = $out_0;
                    break $_$12;
                  }
                  var $add907 = ((HEAPU8[$next_30] & 255) << $bits_26) + $hold_26 | 0;
                  var $next_30 = $next_30 + 1 | 0;
                  var $have_30 = $have_30 - 1 | 0;
                  var $hold_26 = $add907;
                  var $bits_26 = $bits_26 + 8 | 0;
                }
                var $tmp26 = HEAPU16[($132 + ($and889 << 2) + 2 | 0) >> 1];
                if (($tmp26 & 65535) < 16) {
                  var $next_31 = $next_30;
                  var $have_31 = $have_30;
                  var $hold_27 = $hold_26;
                  var $bits_27 = $bits_26;
                  while (1) {
                    var $bits_27;
                    var $hold_27;
                    var $have_31;
                    var $next_31;
                    if (!($bits_27 >>> 0 < $conv893 >>> 0)) {
                      break;
                    }
                    if (($have_31 | 0) == 0) {
                      var $ret_8 = $ret_1_ph;
                      var $next_58 = $next_31;
                      var $have_58 = 0;
                      var $hold_54 = $hold_27;
                      var $bits_54 = $bits_27;
                      var $out_4 = $out_0;
                      break $_$12;
                    }
                    var $add931 = ((HEAPU8[$next_31] & 255) << $bits_27) + $hold_27 | 0;
                    var $next_31 = $next_31 + 1 | 0;
                    var $have_31 = $have_31 - 1 | 0;
                    var $hold_27 = $add931;
                    var $bits_27 = $bits_27 + 8 | 0;
                  }
                  var $shr941 = $hold_27 >>> ($conv893 >>> 0);
                  var $sub944 = $bits_27 - $conv893 | 0;
                  HEAP32[$40 >> 2] = $128 + 1 | 0;
                  HEAP16[($41 + ($128 << 1) | 0) >> 1] = $tmp26;
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
                      if (!($bits_28 >>> 0 < $add962 >>> 0)) {
                        break;
                      }
                      if (($have_32 | 0) == 0) {
                        var $ret_8 = $ret_1_ph;
                        var $next_58 = $next_32;
                        var $have_58 = 0;
                        var $hold_54 = $hold_28;
                        var $bits_54 = $bits_28;
                        var $out_4 = $out_0;
                        break $_$12;
                      }
                      var $add975 = ((HEAPU8[$next_32] & 255) << $bits_28) + $hold_28 | 0;
                      var $next_32 = $next_32 + 1 | 0;
                      var $have_32 = $have_32 - 1 | 0;
                      var $hold_28 = $add975;
                      var $bits_28 = $bits_28 + 8 | 0;
                    }
                    var $shr985 = $hold_28 >>> ($conv893 >>> 0);
                    var $sub988 = $bits_28 - $conv893 | 0;
                    if (($128 | 0) == 0) {
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
                    var $len_0 = HEAP16[($41 + (($128 - 1 | 0) << 1) | 0) >> 1];
                    var $next_35 = $next_32;
                    var $have_35 = $have_32;
                    var $hold_31 = $shr985 >>> 2;
                    var $bits_31 = $sub988 - 2 | 0;
                    var $copy_5 = ($shr985 & 3) + 3 | 0;
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
                      if (!($bits_29 >>> 0 < $add1020 >>> 0)) {
                        break;
                      }
                      if (($have_33 | 0) == 0) {
                        var $ret_8 = $ret_1_ph;
                        var $next_58 = $next_33;
                        var $have_58 = 0;
                        var $hold_54 = $hold_29;
                        var $bits_54 = $bits_29;
                        var $out_4 = $out_0;
                        break $_$12;
                      }
                      var $add1033 = ((HEAPU8[$next_33] & 255) << $bits_29) + $hold_29 | 0;
                      var $next_33 = $next_33 + 1 | 0;
                      var $have_33 = $have_33 - 1 | 0;
                      var $hold_29 = $add1033;
                      var $bits_29 = $bits_29 + 8 | 0;
                    }
                    var $shr1043 = $hold_29 >>> ($conv893 >>> 0);
                    var $len_0 = 0;
                    var $next_35 = $next_33;
                    var $have_35 = $have_33;
                    var $hold_31 = $shr1043 >>> 3;
                    var $bits_31 = (-3 - $conv893 | 0) + $bits_29 | 0;
                    var $copy_5 = ($shr1043 & 7) + 3 | 0;
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
                      if (!($bits_30 >>> 0 < $add1061 >>> 0)) {
                        break;
                      }
                      if (($have_34 | 0) == 0) {
                        var $ret_8 = $ret_1_ph;
                        var $next_58 = $next_34;
                        var $have_58 = 0;
                        var $hold_54 = $hold_30;
                        var $bits_54 = $bits_30;
                        var $out_4 = $out_0;
                        break $_$12;
                      }
                      var $add1074 = ((HEAPU8[$next_34] & 255) << $bits_30) + $hold_30 | 0;
                      var $next_34 = $next_34 + 1 | 0;
                      var $have_34 = $have_34 - 1 | 0;
                      var $hold_30 = $add1074;
                      var $bits_30 = $bits_30 + 8 | 0;
                    }
                    var $shr1084 = $hold_30 >>> ($conv893 >>> 0);
                    var $len_0 = 0;
                    var $next_35 = $next_34;
                    var $have_35 = $have_34;
                    var $hold_31 = $shr1084 >>> 7;
                    var $bits_31 = (-7 - $conv893 | 0) + $bits_30 | 0;
                    var $copy_5 = ($shr1084 & 127) + 11 | 0;
                  }
                  var $copy_5;
                  var $bits_31;
                  var $hold_31;
                  var $have_35;
                  var $next_35;
                  var $len_0;
                  if (($128 + $copy_5 | 0) >>> 0 > $add881 >>> 0) {
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
                    HEAP32[$40 >> 2] = $139 + 1 | 0;
                    HEAP16[($41 + ($139 << 1) | 0) >> 1] = $len_0;
                    if (($dec1111 | 0) == 0) {
                      var $next_29 = $next_35;
                      var $have_29 = $have_35;
                      var $hold_25 = $hold_31;
                      var $bits_25 = $bits_31;
                      continue $_$167;
                    }
                    var $copy_6127 = $dec1111;
                    var $139 = HEAP32[$40 >> 2];
                  }
                }
              } else {
                if ((HEAP32[$mode >> 2] | 0) == 29) {
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
                if (HEAP16[$45 >> 1] << 16 >> 16 == 0) {
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
                if (!(($call1149 | 0) == 0)) {
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
                var $_c = HEAP32[$42 >> 2];
                HEAP32[$46 >> 2] = $_c;
                HEAP32[$47 >> 2] = 6;
                var $add_ptr1159 = $arraydecay864 + (HEAP32[$37 >> 2] << 1) | 0;
                var $143 = HEAP32[$38 >> 2];
                var $call1165 = _inflate_table(2, $add_ptr1159, $143, $42, $47, $arraydecay867);
                if (!(($call1165 | 0) == 0)) {
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
            var $tobool263 = ($71 & 1024 | 0) == 0;
            do {
              if ($tobool263) {
                var $76 = HEAP32[$18 >> 2];
                if (($76 | 0) == 0) {
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
                  if (!($bits_6 >>> 0 < 16)) {
                    break;
                  }
                  if (($have_6 | 0) == 0) {
                    var $ret_8 = $ret_0;
                    var $next_58 = $next_6;
                    var $have_58 = 0;
                    var $hold_54 = $hold_6;
                    var $bits_54 = $bits_6;
                    var $out_4 = $out_0;
                    break $_$12;
                  }
                  var $add279 = ((HEAPU8[$next_6] & 255) << $bits_6) + $hold_6 | 0;
                  var $next_6 = $next_6 + 1 | 0;
                  var $have_6 = $have_6 - 1 | 0;
                  var $hold_6 = $add279;
                  var $bits_6 = $bits_6 + 8 | 0;
                }
                HEAP32[$21 >> 2] = $hold_6;
                var $73 = HEAP32[$18 >> 2];
                if (($73 | 0) == 0) {
                  var $74 = $71;
                } else {
                  var $extra_len = $73 + 20 | 0;
                  HEAP32[$extra_len >> 2] = $hold_6;
                  var $74 = HEAP32[$17 >> 2];
                }
                var $74;
                if (($74 & 512 | 0) == 0) {
                  var $next_7 = $next_6;
                  var $have_7 = $have_6;
                  var $hold_7 = 0;
                  var $bits_7 = 0;
                  break;
                }
                HEAP8[$arrayidx] = $hold_6 & 255;
                HEAP8[$arrayidx40] = $hold_6 >>> 8 & 255;
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
            if ((HEAP32[$24 >> 2] | 0) == 0) {
              var $next_22 = $next_21;
              var $have_22 = $have_21;
              var $hold_18 = $hold_17;
              var $bits_18 = $bits_17;
              while (1) {
                var $bits_18;
                var $hold_18;
                var $have_22;
                var $next_22;
                if (!($bits_18 >>> 0 < 3)) {
                  break;
                }
                if (($have_22 | 0) == 0) {
                  var $ret_8 = $ret_0;
                  var $next_58 = $next_22;
                  var $have_58 = 0;
                  var $hold_54 = $hold_18;
                  var $bits_54 = $bits_18;
                  var $out_4 = $out_0;
                  break $_$12;
                }
                var $add641 = ((HEAPU8[$next_22] & 255) << $bits_18) + $hold_18 | 0;
                var $next_22 = $next_22 + 1 | 0;
                var $have_22 = $have_22 - 1 | 0;
                var $hold_18 = $add641;
                var $bits_18 = $bits_18 + 8 | 0;
              }
              HEAP32[$24 >> 2] = $hold_18 & 1;
              var $and655 = $hold_18 >>> 1 & 3;
              do {
                if (($and655 | 0) == 0) {
                  HEAP32[$mode >> 2] = 13;
                } else if (($and655 | 0) == 1) {
                  _fixedtables($4);
                  HEAP32[$mode >> 2] = 19;
                  if (!$cmp660) {
                    break;
                  }
                  var $ret_8 = $ret_0;
                  var $next_58 = $next_22;
                  var $have_58 = $have_22;
                  var $hold_54 = $hold_18 >>> 3;
                  var $bits_54 = $bits_18 - 3 | 0;
                  var $out_4 = $out_0;
                  break $_$12;
                } else if (($and655 | 0) == 2) {
                  HEAP32[$mode >> 2] = 16;
                } else if (($and655 | 0) == 3) {
                  HEAP32[$msg >> 2] = STRING_TABLE.__str6103 | 0;
                  HEAP32[$mode >> 2] = 29;
                }
              } while (0);
              var $ret_0_be = $ret_0;
              var $next_0_be = $next_22;
              var $put_0_be = $put_0;
              var $have_0_be = $have_22;
              var $left_0_be = $left_0;
              var $hold_0_be = $hold_18 >>> 3;
              var $bits_0_be = $bits_18 - 3 | 0;
              var $out_0_be = $out_0;
              __label__ = 268;
              break;
            }
            var $and619 = $bits_17 & 7;
            HEAP32[$mode >> 2] = 26;
            var $ret_0_be = $ret_0;
            var $next_0_be = $next_21;
            var $put_0_be = $put_0;
            var $have_0_be = $have_21;
            var $left_0_be = $left_0;
            var $hold_0_be = $hold_17 >>> ($and619 >>> 0);
            var $bits_0_be = $bits_17 - $and619 | 0;
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
            if (($77 & 1024 | 0) == 0) {
              var $next_10 = $next_8;
              var $have_10 = $have_8;
              var $88 = $77;
            } else {
              var $78 = HEAPU32[$21 >> 2];
              var $copy_0 = $78 >>> 0 > $have_8 >>> 0 ? $have_8 : $78;
              if (($copy_0 | 0) == 0) {
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
                    var $80 = HEAP32[($79 + 16 | 0) >> 2];
                    if (($80 | 0) == 0) {
                      var $83 = $77;
                      break;
                    }
                    var $sub341 = HEAP32[($79 + 20 | 0) >> 2] - $78 | 0;
                    var $add_ptr = $80 + $sub341 | 0;
                    var $82 = HEAPU32[($79 + 24 | 0) >> 2];
                    var $cond351 = ($sub341 + $copy_0 | 0) >>> 0 > $82 >>> 0 ? $82 - $sub341 | 0 : $copy_0;
                    _memcpy($add_ptr, $next_8, $cond351, 1);
                    var $83 = HEAP32[$17 >> 2];
                  }
                } while (0);
                var $83;
                if (!(($83 & 512 | 0) == 0)) {
                  var $84 = HEAP32[$16 >> 2];
                  var $call358 = _crc32($84, $next_8, $copy_0);
                  HEAP32[$16 >> 2] = $call358;
                }
                var $sub361 = $have_8 - $copy_0 | 0;
                var $add_ptr362 = $next_8 + $copy_0 | 0;
                var $sub364 = HEAP32[$21 >> 2] - $copy_0 | 0;
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
              if (!(($87 | 0) == 0)) {
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
            if ($have_38 >>> 0 > 5 & $left_0 >>> 0 > 257) {
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
              if (!((HEAP32[$mode >> 2] | 0) == 11)) {
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
            var $sub1213 = (1 << HEAP32[$26 >> 2]) - 1 | 0;
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
              var $tmp22 = HEAPU8[$152 + ($and1214 << 2) + 1 | 0];
              var $conv1218 = $tmp22 & 255;
              if (!($conv1218 >>> 0 > $bits_35 >>> 0)) {
                break;
              }
              if (($have_39 | 0) == 0) {
                var $ret_8 = $ret_3;
                var $next_58 = $next_39;
                var $have_58 = 0;
                var $hold_54 = $hold_35;
                var $bits_54 = $bits_35;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add1232 = ((HEAPU8[$next_39] & 255) << $bits_35) + $hold_35 | 0;
              var $next_39 = $next_39 + 1 | 0;
              var $have_39 = $have_39 - 1 | 0;
              var $hold_35 = $add1232;
              var $bits_35 = $bits_35 + 8 | 0;
            }
            var $tmp21 = HEAPU8[$152 + ($and1214 << 2) | 0];
            var $tmp23 = HEAPU16[($152 + ($and1214 << 2) + 2 | 0) >> 1];
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
                if (!(($conv1237 & 240 | 0) == 0)) {
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
                var $sub1255 = (1 << ($conv1218 + $conv1237 | 0)) - 1 | 0;
                var $next_40 = $next_39;
                var $have_40 = $have_39;
                var $hold_36 = $hold_35;
                var $bits_36 = $bits_35;
                while (1) {
                  var $bits_36;
                  var $hold_36;
                  var $have_40;
                  var $next_40;
                  var $add1260 = (($hold_36 & $sub1255) >>> ($conv1218 >>> 0)) + $conv1248 | 0;
                  var $tmp19 = HEAPU8[$152 + ($add1260 << 2) + 1 | 0];
                  if (!((($tmp19 & 255) + $conv1218 | 0) >>> 0 > $bits_36 >>> 0)) {
                    break;
                  }
                  if (($have_40 | 0) == 0) {
                    var $ret_8 = $ret_3;
                    var $next_58 = $next_40;
                    var $have_58 = 0;
                    var $hold_54 = $hold_36;
                    var $bits_54 = $bits_36;
                    var $out_4 = $out_0;
                    break $_$12;
                  }
                  var $add1281 = ((HEAPU8[$next_40] & 255) << $bits_36) + $hold_36 | 0;
                  var $next_40 = $next_40 + 1 | 0;
                  var $have_40 = $have_40 - 1 | 0;
                  var $hold_36 = $add1281;
                  var $bits_36 = $bits_36 + 8 | 0;
                }
                var $tmp20 = HEAP16[($152 + ($add1260 << 2) + 2 | 0) >> 1];
                var $tmp18 = HEAP8[$152 + ($add1260 << 2) | 0];
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
            HEAP32[$25 >> 2] = $155 + $conv1302 | 0;
            HEAP32[$21 >> 2] = $here_211_0 & 65535;
            var $conv1317 = $here_09_0 & 255;
            if ($here_09_0 << 24 >> 24 == 0) {
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
            if (!(($conv1317 & 32 | 0) == 0)) {
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
            if (($conv1317 & 64 | 0) == 0) {
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
            var $tobool376 = ($89 & 2048 | 0) == 0;
            do {
              if ($tobool376) {
                var $98 = HEAP32[$18 >> 2];
                if (($98 | 0) == 0) {
                  var $next_12 = $next_11;
                  var $have_12 = $have_11;
                  break;
                }
                var $name428 = $98 + 28 | 0;
                HEAP32[$name428 >> 2] = 0;
                var $next_12 = $next_11;
                var $have_12 = $have_11;
              } else {
                if (($have_11 | 0) == 0) {
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
                  var $90 = HEAP8[$next_11 + $copy_1 | 0];
                  var $91 = HEAP32[$18 >> 2];
                  var $cmp386 = ($91 | 0) == 0;
                  do {
                    if (!$cmp386) {
                      var $name = $91 + 28 | 0;
                      if ((HEAP32[$name >> 2] | 0) == 0) {
                        break;
                      }
                      var $93 = HEAPU32[$21 >> 2];
                      if (!($93 >>> 0 < HEAPU32[($91 + 32 | 0) >> 2] >>> 0)) {
                        break;
                      }
                      HEAP32[$21 >> 2] = $93 + 1 | 0;
                      HEAP8[HEAP32[$name >> 2] + $93 | 0] = $90;
                    }
                  } while (0);
                  var $tobool405 = $90 << 24 >> 24 != 0;
                  if (!($tobool405 & $inc >>> 0 < $have_11 >>> 0)) {
                    break;
                  }
                  var $copy_1 = $inc;
                }
                if (!((HEAP32[$17 >> 2] & 512 | 0) == 0)) {
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
            if (($156 | 0) == 0) {
              var $next_44 = $next_42;
              var $have_44 = $have_42;
              var $hold_40 = $hold_38;
              var $bits_40 = $bits_38;
              var $160 = HEAP32[$21 >> 2];
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
                if (!($bits_39 >>> 0 < $156 >>> 0)) {
                  break;
                }
                if (($have_43 | 0) == 0) {
                  var $ret_8 = $ret_4;
                  var $next_58 = $next_43;
                  var $have_58 = 0;
                  var $hold_54 = $hold_39;
                  var $bits_54 = $bits_39;
                  var $out_4 = $out_0;
                  break $_$12;
                }
                var $add1363 = ((HEAPU8[$next_43] & 255) << $bits_39) + $hold_39 | 0;
                var $next_43 = $next_43 + 1 | 0;
                var $have_43 = $have_43 - 1 | 0;
                var $hold_39 = $add1363;
                var $bits_39 = $bits_39 + 8 | 0;
              }
              var $add1375 = HEAP32[$21 >> 2] + (((1 << $156) - 1 | 0) & $hold_39) | 0;
              HEAP32[$21 >> 2] = $add1375;
              var $add1385 = HEAP32[$25 >> 2] + $156 | 0;
              HEAP32[$25 >> 2] = $add1385;
              var $next_44 = $next_43;
              var $have_44 = $have_43;
              var $hold_40 = $hold_39 >>> ($156 >>> 0);
              var $bits_40 = $bits_39 - $156 | 0;
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
            var $tobool436 = (HEAP32[$17 >> 2] & 4096 | 0) == 0;
            do {
              if ($tobool436) {
                var $108 = HEAP32[$18 >> 2];
                if (($108 | 0) == 0) {
                  var $next_14 = $next_13;
                  var $have_14 = $have_13;
                  break;
                }
                var $comment492 = $108 + 36 | 0;
                HEAP32[$comment492 >> 2] = 0;
                var $next_14 = $next_13;
                var $have_14 = $have_13;
              } else {
                if (($have_13 | 0) == 0) {
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
                  var $100 = HEAP8[$next_13 + $copy_2 | 0];
                  var $101 = HEAP32[$18 >> 2];
                  var $cmp447 = ($101 | 0) == 0;
                  do {
                    if (!$cmp447) {
                      var $comment = $101 + 36 | 0;
                      if ((HEAP32[$comment >> 2] | 0) == 0) {
                        break;
                      }
                      var $103 = HEAPU32[$21 >> 2];
                      if (!($103 >>> 0 < HEAPU32[($101 + 40 | 0) >> 2] >>> 0)) {
                        break;
                      }
                      HEAP32[$21 >> 2] = $103 + 1 | 0;
                      HEAP8[HEAP32[$comment >> 2] + $103 | 0] = $100;
                    }
                  } while (0);
                  var $tobool467 = $100 << 24 >> 24 != 0;
                  if (!($tobool467 & $inc443 >>> 0 < $have_13 >>> 0)) {
                    break;
                  }
                  var $copy_2 = $inc443;
                }
                if (!((HEAP32[$17 >> 2] & 512 | 0) == 0)) {
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
            var $sub1393 = (1 << HEAP32[$47 >> 2]) - 1 | 0;
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
              var $tmp16 = HEAPU8[$162 + ($and1394 << 2) + 1 | 0];
              var $conv1398 = $tmp16 & 255;
              if (!($conv1398 >>> 0 > $bits_41 >>> 0)) {
                break;
              }
              if (($have_45 | 0) == 0) {
                var $ret_8 = $ret_5_ph;
                var $next_58 = $next_45;
                var $have_58 = 0;
                var $hold_54 = $hold_41;
                var $bits_54 = $bits_41;
                var $out_4 = $out_0;
                break $_$12;
              }
              var $add1412 = ((HEAPU8[$next_45] & 255) << $bits_41) + $hold_41 | 0;
              var $next_45 = $next_45 + 1 | 0;
              var $have_45 = $have_45 - 1 | 0;
              var $hold_41 = $add1412;
              var $bits_41 = $bits_41 + 8 | 0;
            }
            var $tmp15 = HEAPU8[$162 + ($and1394 << 2) | 0];
            var $tmp17 = HEAPU16[($162 + ($and1394 << 2) + 2 | 0) >> 1];
            var $conv1418 = $tmp15 & 255;
            if (($conv1418 & 240 | 0) == 0) {
              var $conv1425 = $tmp17 & 65535;
              var $sub1432 = (1 << ($conv1398 + $conv1418 | 0)) - 1 | 0;
              var $next_46 = $next_45;
              var $have_46 = $have_45;
              var $hold_42 = $hold_41;
              var $bits_42 = $bits_41;
              while (1) {
                var $bits_42;
                var $hold_42;
                var $have_46;
                var $next_46;
                var $add1437 = (($hold_42 & $sub1432) >>> ($conv1398 >>> 0)) + $conv1425 | 0;
                var $tmp13 = HEAPU8[$162 + ($add1437 << 2) + 1 | 0];
                if (!((($tmp13 & 255) + $conv1398 | 0) >>> 0 > $bits_42 >>> 0)) {
                  break;
                }
                if (($have_46 | 0) == 0) {
                  var $ret_8 = $ret_5_ph;
                  var $next_58 = $next_46;
                  var $have_58 = 0;
                  var $hold_54 = $hold_42;
                  var $bits_54 = $bits_42;
                  var $out_4 = $out_0;
                  break $_$12;
                }
                var $add1458 = ((HEAPU8[$next_46] & 255) << $bits_42) + $hold_42 | 0;
                var $next_46 = $next_46 + 1 | 0;
                var $have_46 = $have_46 - 1 | 0;
                var $hold_42 = $add1458;
                var $bits_42 = $bits_42 + 8 | 0;
              }
              var $tmp14 = HEAP16[($162 + ($add1437 << 2) + 2 | 0) >> 1];
              var $tmp12 = HEAP8[$162 + ($add1437 << 2) | 0];
              var $shr1466 = $hold_42 >>> ($conv1398 >>> 0);
              var $sub1469 = $bits_42 - $conv1398 | 0;
              var $add1475 = HEAP32[$25 >> 2] + $conv1398 | 0;
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
              var $next_47 = $next_45;
              var $have_47 = $have_45;
              var $hold_43 = $hold_41;
              var $bits_43 = $bits_41;
              var $here_09_1 = $tmp15;
              var $here_110_1 = $tmp16;
              var $here_211_1 = $tmp17;
              var $166 = HEAP32[$25 >> 2];
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
            HEAP32[$25 >> 2] = $166 + $conv1479 | 0;
            var $conv1491 = $here_09_1 & 255;
            if (($conv1491 & 64 | 0) == 0) {
              HEAP32[$30 >> 2] = $here_211_1 & 65535;
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
            var $tobool499 = ($109 & 512 | 0) == 0;
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
                  if (!($bits_12 >>> 0 < 16)) {
                    break;
                  }
                  if (($have_16 | 0) == 0) {
                    var $ret_8 = $ret_0;
                    var $next_58 = $next_16;
                    var $have_58 = 0;
                    var $hold_54 = $hold_12;
                    var $bits_54 = $bits_12;
                    var $out_4 = $out_0;
                    break $_$12;
                  }
                  var $add515 = ((HEAPU8[$next_16] & 255) << $bits_12) + $hold_12 | 0;
                  var $next_16 = $next_16 + 1 | 0;
                  var $have_16 = $have_16 - 1 | 0;
                  var $hold_12 = $add515;
                  var $bits_12 = $bits_12 + 8 | 0;
                }
                if (($hold_12 | 0) == (HEAP32[$16 >> 2] & 65535 | 0)) {
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
            if (!(($112 | 0) == 0)) {
              HEAP32[($112 + 44 | 0) >> 2] = $109 >>> 9 & 1;
              var $done543 = HEAP32[$18 >> 2] + 48 | 0;
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
            if (($167 | 0) == 0) {
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
                if (!($bits_45 >>> 0 < $167 >>> 0)) {
                  break;
                }
                if (($have_49 | 0) == 0) {
                  var $ret_8 = $ret_6;
                  var $next_58 = $next_49;
                  var $have_58 = 0;
                  var $hold_54 = $hold_45;
                  var $bits_54 = $bits_45;
                  var $out_4 = $out_0;
                  break $_$12;
                }
                var $add1524 = ((HEAPU8[$next_49] & 255) << $bits_45) + $hold_45 | 0;
                var $next_49 = $next_49 + 1 | 0;
                var $have_49 = $have_49 - 1 | 0;
                var $hold_45 = $add1524;
                var $bits_45 = $bits_45 + 8 | 0;
              }
              var $add1536 = HEAP32[$30 >> 2] + (((1 << $167) - 1 | 0) & $hold_45) | 0;
              HEAP32[$30 >> 2] = $add1536;
              var $add1546 = HEAP32[$25 >> 2] + $167 | 0;
              HEAP32[$25 >> 2] = $add1546;
              var $next_50 = $next_49;
              var $have_50 = $have_49;
              var $hold_46 = $hold_45 >>> ($167 >>> 0);
              var $bits_46 = $bits_45 - $167 | 0;
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
            if (($left_0 | 0) == 0) {
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
                var $cmp1561 = $sub1560 >>> 0 > HEAPU32[$31 >> 2] >>> 0;
                do {
                  if ($cmp1561) {
                    if ((HEAP32[$32 >> 2] | 0) == 0) {
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
                if ($sub1560 >>> 0 > $174 >>> 0) {
                  var $sub1574 = $sub1560 - $174 | 0;
                  var $from_0 = HEAP32[$34 >> 2] + (HEAP32[$35 >> 2] - $sub1574 | 0) | 0;
                  var $copy_7 = $sub1574;
                } else {
                  var $from_0 = HEAP32[$34 >> 2] + ($174 - $sub1560 | 0) | 0;
                  var $copy_7 = $sub1560;
                }
                var $copy_7;
                var $from_0;
                var $178 = HEAPU32[$21 >> 2];
                if (!($copy_7 >>> 0 > $178 >>> 0)) {
                  var $from_1 = $from_0;
                  var $copy_8 = $copy_7;
                  var $180 = $178;
                  break;
                }
                var $from_1 = $from_0;
                var $copy_8 = $178;
                var $180 = $178;
              } else {
                var $179 = HEAP32[$21 >> 2];
                var $from_1 = $put_0 + (-$171 | 0) | 0;
                var $copy_8 = $179;
                var $180 = $179;
              }
            } while (0);
            var $180;
            var $copy_8;
            var $from_1;
            var $copy_9 = $copy_8 >>> 0 > $left_0 >>> 0 ? $left_0 : $copy_8;
            HEAP32[$21 >> 2] = $180 - $copy_9 | 0;
            var $181 = $copy_8 ^ -1;
            var $182 = $left_0 ^ -1;
            var $umax = $181 >>> 0 > $182 >>> 0 ? $181 : $182;
            var $from_2 = $from_1;
            var $put_1 = $put_0;
            var $copy_10 = $copy_9;
            while (1) {
              var $copy_10;
              var $put_1;
              var $from_2;
              var $184 = HEAP8[$from_2];
              HEAP8[$put_1] = $184;
              var $dec1605 = $copy_10 - 1 | 0;
              if (($dec1605 | 0) == 0) {
                break;
              }
              var $from_2 = $from_2 + 1 | 0;
              var $put_1 = $put_1 + 1 | 0;
              var $copy_10 = $dec1605;
            }
            var $sub1598 = $left_0 - $copy_9 | 0;
            var $scevgep632 = $put_0 + ($umax ^ -1) | 0;
            if (!((HEAP32[$21 >> 2] | 0) == 0)) {
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
        var $ret_0 = $ret_0_be;
        var $next_0 = $next_0_be;
        var $put_0 = $put_0_be;
        var $have_0 = $have_0_be;
        var $left_0 = $left_0_be;
        var $hold_0 = $hold_0_be;
        var $bits_0 = $bits_0_be;
        var $out_0 = $out_0_be;
        var $49 = HEAP32[$mode >> 2];
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
      var $tobool1755 = (HEAP32[$35 >> 2] | 0) == 0;
      do {
        if ($tobool1755) {
          if (!(HEAPU32[$mode >> 2] >>> 0 < 26)) {
            __label__ = 300;
            break;
          }
          if (($out_4 | 0) == (HEAP32[$avail_out >> 2] | 0)) {
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
          if (($call1765 | 0) == 0) {
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
      var $add1775 = ($10 - $202 | 0) + HEAP32[$total_in >> 2] | 0;
      HEAP32[$total_in >> 2] = $add1775;
      var $add1777 = HEAP32[$total_out >> 2] + $sub1774 | 0;
      HEAP32[$total_out >> 2] = $add1777;
      var $add1779 = HEAP32[$36 >> 2] + $sub1774 | 0;
      HEAP32[$36 >> 2] = $add1779;
      var $tobool1783 = ($out_4 | 0) == ($203 | 0);
      if (!((HEAP32[$15 >> 2] | 0) == 0 | $tobool1783)) {
        var $209 = HEAP32[$16 >> 2];
        var $add_ptr1791 = HEAP32[$next_out >> 2] + (-$sub1774 | 0) | 0;
        if ((HEAP32[$17 >> 2] | 0) == 0) {
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
      var $cond1807 = (HEAP32[$24 >> 2] | 0) != 0 ? 64 : 0;
      var $213 = HEAP32[$mode >> 2];
      var $cond1812 = ($213 | 0) == 11 ? 128 : 0;
      if (($213 | 0) == 19) {
        var $214 = 256;
      } else {
        var $phitmp = ($213 | 0) == 14 ? 256 : 0;
        var $214 = $phitmp;
      }
      var $214;
      var $add1821 = (($cond1807 + HEAP32[$13 >> 2] | 0) + $cond1812 | 0) + $214 | 0;
      HEAP32[($strm + 44 | 0) >> 2] = $add1821;
      var $ret_9 = (($10 | 0) == ($202 | 0) & $tobool1783 | ($flush | 0) == 4) & ($ret_8 | 0) == 0 ? -5 : $ret_8;
      var $retval_0 = $ret_9;
    }
  } while (0);
  var $retval_0;
  STACKTOP = __stackBase__;
  return $retval_0;
  return null;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["f", "g", "h", "py", "r", "t", "f2", "f3", "llvm3_1", "_inflate"]
