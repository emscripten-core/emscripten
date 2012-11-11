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
  something();
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
  z(($f | 0) / 55 & 255);
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
    if ($i >= $N) {
      __label__ = 3;
      break;
    }
    somethingElse();
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
    }
    __label__ = 38;
  } while (0);
  if (__label__ == 38) {
    var $79 = $_pr6;
  }
  pause(9);
  var $cmp70 = ($call69 | 0) != 0;
  pause(10);
  while (check()) {
    if ($i < $N) {
      callOther();
      break;
    }
    somethingElse();
    if ($i1 < $N) {
      callOther();
      continue;
    }
    somethingElse();
    if ($i2 >= $N) {
      somethingElse();
      break;
    }
    callOther();
    if ($i3 >= $N) {
      somethingElse();
      continue;
    }
    callOther();
    if ($i4 < $N) {
      callOther();
      break;
    }
    somethingElse();
    continue;
  }
}
function innerShouldAlsoBeHoisted() {
  function hoisting() {
    if ($i < $N) {
      callOther();
    }
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
    if (!$cmp) {
      if (something()) {
        __label__ = 3;
        break;
      }
      more();
      break;
    }
    __label__ = 3;
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
    }
    inc();
  }
  pause(999);
  $while_body$$while_end$31 : do {
    if ($cmp3) {
      var $6 = $5;
      while (1) {
        var $6;
        $iter = $6 + 3;
        if (FHEAP[$iter + 1] >= $pct_addr) {
          var $_lcssa = $iter;
          break $while_body$$while_end$31;
        }
        var $6 = $iter;
      }
    } else {
      var $_lcssa = $5;
    }
  } while (0);
  var $_lcssa;
  cheez();
}
function notComps() {
  if (HEAP32[$incdec_ptr71_i + 8 >> 2] != 0) {
    shoo();
  }
}
function tricky() {
  var $conv642 = $conv6374 - (($132 << 16 >> 16 | 0) / 2 & -1) & 65535;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["abc", "xyz", "xyz2", "expr", "loopy", "bits", "maths", "hoisting", "demangle", "lua", "moreLabels", "notComps", "tricky"]

