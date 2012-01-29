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
  var oneUse = glob;
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
  var $8 = $4 + 12;
  HEAP[$8] = $7;
}
function otherPy() {
  var $4 = HEAP[__PyThreadState_Current];
  var $5 = $4 + 12;
  var $7 = HEAP[$5] + 1;
  var $8 = $4 + 12;
  HEAP[$8] = $7;
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
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["f", "g", "h", "py", "r", "t", "f2"]

