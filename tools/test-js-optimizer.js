var c;
var b = 5, x = void 0, y = null, t = !0;
var s = !1, s2 = !1, s3 = !0;
var block = [x];
function abc() {
  var cheez = [x];
  var fleefl;
  cheez = 10;
  fleefl = x;
  fleefl = cheez = x;
  var waka = x, flake = x, marfoosh = x;
  var waka2 = 5, flake2 = x, marfoosh2 = x;
  var waka3 = x, flake3 = 5, marfoosh3 = x;
  var waka4 = x, flake4 = x, marfoosh4 = 5;
  var test1 = t, test2 = s, test3 = y, test4 = z, test5 = b, test6 = x;
  s3 = 9;
}
function xyz() {
  var x = 52;
  var cheez = x;
  var t = 22;
  function inner1(c) {
    var i = x, j = y, k = 5, l = t, m = s2;
    var s2 = 8;
  }
  var inner2 = function(d) {
    var i = x, j = y, k = 5, l = t, m = s2;
    var s2 = 8;
  };
  var patama = s3;
}
function xyz2(x) {
  var cheez = x;
}
zzz = function(nada) {
  var cheez = x;
};
function expr() {
  if (!($0 < $1)) print("hi");
}
function loopy() {
  $while_body$2: while(1) { 
    $ok=1;
    $for_cond$4: while(1) { 
      if ($ok) break $for_cond$4;
      var $inc=$ok+1;
      if ($inc == 9999) break $while_body$2; // this forces a label to remain on the outer loop
    }
    continue $while_body$2;
  }
  next();
  b$while_body$2: while(1) { 
    $ok=1;
    b$for_cond$4: while(1) {
      if ($ok) break b$for_cond$4;
      var $inc=$ok+1;
    }
    continue b$while_body$2;
  }
  next();
  $once: do {
    if (!$ok) break $once; // forces the entire one-time do to remain (but unlabelled)
    something();
  } while(0);
  next();
  b$once: do {
    while (more()) {
      if (!$ok) break b$once; // forces the entire one-time do to remain, with label
    }
    something();
  } while(0);
  next();
  c$once: do {
    something();
  } while(0);
}
function ignoreLoopy() {
  b$for_cond$4: while(1) {
    if ($ok) break b$for_cond$4;
    var $inc=$ok+1;
  }
}
function bits() { // TODO: optimize this!
  print((($s & 65535) + ((($f & 65535) << 16 >> 16) * (($f & 65535) << 16 >> 16) | 0 | 0) % 256 | 0) & 65535);
}
function maths() {
  check(5+12);
  check(90+3+2);
  __ZN6b2Vec2C1Ev(((((((($this1 + 20 | 0 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0);
}
function hoisting() {
  if ($i < $N) {
    __label__ = 2;
  }
  if (__label__ == 2) {
    callOther();
  }
// ok /*
  pause(1);
  if ($i < $N) {
    __label__ = 2;
  } else {
    __label__ = 3;
  }
  $for_body3$$for_end$5 : do {
    if (__label__ == 2) {
      while(true) { break $for_body3$$for_end$5 }
      callOther();
    }
  } while (0);
  pause(2);
  if ($i < $N) {
    __label__ = 2;
  } else {
    __label__ = 3;
  }
  cheez: do {
    if (__label__ == 2) {
      if (callOther()) break cheez;
    }
  } while (0);
  pause(3);
  if ($i < $N) {
    __label__ = 2;
  } else {
    __label__ = 3;
  }
  if (__label__ == 2) {
    callOther();
  }
  pause(4);
  if ($i < $N) {
    __label__ = 2;
  } else {
    __label__ = 3;
  }
  if (__label__ == 2) {
    callOther();
  } else if (__label__ == 3) {
    somethingElse();
  }
  pause(5);
  if ($i < $N) {
    __label__ = 2;
  } else {
    __label__ = 3;
  }
  if (__label__ == 55) {
    callOther();
  } else if (__label__ == 3) {
    somethingElse();
  }
  pause(6);
  if ($i < $N) {
    __label__ = 2;
  } else {
    __label__ = 3;
  }
  if (__label__ == 3) {
    somethingElse();
  }
}
var FS = {
  absolutePath: function(relative, base) { // Don't touch this!
    if (typeof relative !== 'string') return null;
    if (base === undefined) base = FS.currentPath;
    if (relative && relative[0] == '/') base = '';
    var full = base + '/' + relative;
    var parts = full.split('/').reverse();
    var absolute = [''];
    while (parts.length) {
      var part = parts.pop();
      if (part == '' || part == '.') {
        // Nothing.
      } else if (part == '..') {
        if (absolute.length > 1) absolute.pop();
      } else {
        absolute.push(part);
      }
    }
    return absolute.length == 1 ? '/' : absolute.join('/');
  }
}
function sleep() {
  while (Date.now() - start < msec) {
    // Do nothing. This empty block should remain untouched (c.f. _usleep)
  }
  return 0;
}
function demangle($cmp) {
  if ($cmp) {
    __label__ = 3;
  } else {
    __label__ = 1;
  }
  $if_then$$lor_lhs_false$2 : do {
    if (__label__ == 1) {
      if (something()) {
        __label__ = 3;
        break $if_then$$lor_lhs_false$2;
      }
      more();
      break $if_then$$lor_lhs_false$2;
    }
  } while (0);
  $if_then$$return$6 : do {
    if (__label__ == 3) {
      final();
    }
  } while (0);
}
function lua() {
  $5$98 : while (1) {
    if ($14) {
      __label__ = 3;
    } else {
      __label__ = 4;
    }
    $15$$16$101 : do {
      if (__label__ == 3) {} else if (__label__ == 4) {
        var $17 = $i;
        var $18 = $3;
        var $19 = $18 + ($17 << 2) | 0;
        var $20 = HEAP32[$19 >> 2];
        var $21 = $20 + 1 | 0;
        var $22 = HEAP8[$21];
        var $23 = $22 << 24 >> 24;
        break $15$$16$101;
      }
    } while(0);
  }
  pause();
  if ($1435 == 0) {
    __label__ = 176;
  } else if ($1435 == 1) {} else {
    __label__ = 180;
  }
  pause();
  if ($1435 == 0) {
    __label__ = 176;
  } else if ($1435 == 1) {}
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["abc", "xyz", "xyz2", "expr", "loopy", "bits", "maths", "hoisting", "demangle", "lua"]
