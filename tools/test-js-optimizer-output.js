var c;
var b = 5;
var s3 = !0;
var block = [ void 0 ];
function abc() {
  var cheez = [ void 0 ];
  var fleefl;
  cheez = 10;
  var waka, flake, marfoosh;
  var waka2 = 5, flake2, marfoosh2;
  var waka3, flake3 = 5, marfoosh3;
  var waka4, flake4, marfoosh4 = 5;
  var test1 = !0, test2 = !1, test3 = null, test4 = z, test5 = b, test6;
  s3 = 9;
}
function xyz() {
  var x = 52;
  var cheez = x;
  var t = 22;
  function inner1(c) {
    var i = x, j = null, k = 5, l = t, m = s2;
    var s2 = 8;
  }
  var inner2 = (function(d) {
    var i = x, j = null, k = 5, l = t, m = s2;
    var s2 = 8;
  });
  var patama = s3;
}
function xyz2(x) {
  var cheez = x;
}
zzz = (function(nada) {
  var cheez;
});
function expr() {
  if ($0 >= $1) print("hi");
}
function loopy() {
  $while_body$2 : while (1) {
    $ok = 1;
    while (1) {
      if ($ok) break;
      var $inc = $ok + 1;
      if ($inc == 9999) break $while_body$2;
    }
    continue;
  }
  next();
  while (1) {
    $ok = 1;
    while (1) {
      if ($ok) break;
      var $inc = $ok + 1;
    }
    continue;
  }
  next();
  do {
    if (!$ok) break;
    something();
  } while (0);
  next();
  b$once : do {
    while (more()) {
      if (!$ok) break b$once;
    }
    something();
  } while (0);
  next();
  {
    something();
  }
}
function ignoreLoopy() {
  b$for_cond$4 : while (1) {
    if ($ok) break b$for_cond$4;
    var $inc = $ok + 1;
  }
}
function bits() {
  print(($s & 65535) + (($f & 65535) << 16 >> 16) * (($f & 65535) << 16 >> 16) % 256 & 65535);
  z(HEAP32[$id + 40 >> 2]);
  z($f << 2);
  z($f | 255);
  z($f & 255);
  z($f ^ 1);
  z($f << 2);
  z($f * 100 << 2);
  z($f % 2 | 255);
  z($f / 55 & 255);
  z($f - 22 ^ 1);
  z($f + 15 << 2);
}
function maths() {
  check(17);
  check(95);
  __ZN6b2Vec2C1Ev($this1 + 76 | 0);
}
function hoisting() {
  if ($i < $N) {
    callOther();
  }
  pause(1);
  $for_body3$$for_end$5 : do {
    if ($i < $N) {
      while (true) {
        break $for_body3$$for_end$5;
      }
      callOther();
    }
  } while (0);
  pause(2);
  do {
    if ($i < $N) {
      if (callOther()) break;
    }
  } while (0);
  pause(3);
  if ($i < $N) {
    callOther();
  }
  pause(4);
  if ($i < $N) {
    callOther();
  } else {
    somethingElse();
  }
  pause(5);
  if ($i < $N) {
    __label__ = 2;
  } else {
    somethingElse();
  }
  if (__label__ == 55) {
    callOther();
  }
  pause(6);
  if ($i >= $N) {
    somethingElse();
  }
  pause(7);
  while (1) {
    if ($i < $N) {
      somethingElse();
    } else {
      __label__ = 3;
      break;
    }
    if ($i < $N) {
      somethingElse();
    }
    nothing();
  }
  pause(8);
  var $cmp95 = $69 == -1;
  do {
    if ($cmp95) {
      if (!$cmp103) {
        __label__ = 38;
        break;
      }
      if (!$cmp106) {
        __label__ = 38;
        break;
      }
      __label__ = 39;
      break;
    } else {
      __label__ = 38;
    }
  } while (0);
  if (__label__ == 38) {
    var $79 = $_pr6;
  }
}
var FS = {
  absolutePath: (function(relative, base) {
    if (typeof relative !== "string") return null;
    if (base === undefined) base = FS.currentPath;
    if (relative && relative[0] == "/") base = "";
    var full = base + "/" + relative;
    var parts = full.split("/").reverse();
    var absolute = [ "" ];
    while (parts.length) {
      var part = parts.pop();
      if (part == "" || part == ".") {} else if (part == "..") {
        if (absolute.length > 1) absolute.pop();
      } else {
        absolute.push(part);
      }
    }
    return absolute.length == 1 ? "/" : absolute.join("/");
  })
};
function sleep() {
  while (Date.now() - start < msec) {}
  return 0;
}
function demangle($cmp) {
  do {
    if ($cmp) {
      __label__ = 3;
    } else {
      if (something()) {
        __label__ = 3;
        break;
      }
      more();
      break;
    }
  } while (0);
  if (__label__ == 3) {
    final();
  }
}
function lua() {
  while (1) {
    do {
      if (!$14) {
        var $17 = $i;
        var $18 = $3;
        var $19 = $18 + ($17 << 2) | 0;
        var $20 = HEAP32[$19 >> 2];
        var $21 = $20 + 1 | 0;
        var $22 = HEAP8[$21];
        var $23 = $22 << 24 >> 24;
        break;
      }
    } while (0);
  }
  pause();
  if ($1435 == 0) {
    __label__ = 176;
    cheez();
  } else if ($1435 != 1) {
    __label__ = 180;
    cheez();
  }
  pause();
  if ($1435 == 0) {
    __label__ = 176;
    cheez();
  }
}
function moreLabels() {
  while (1) {
    if (!$cmp) {
      break;
    }
    if ($cmp1) {
      break;
    } else {
      inc();
    }
  }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["abc", "xyz", "xyz2", "expr", "loopy", "bits", "maths", "hoisting", "demangle", "lua", "moreLabels"]
