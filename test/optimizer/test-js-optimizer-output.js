function expr() {
 if ($0 >= $1) print("hi");
}
function loopy() {
 $while_body$2 : while (1) {
  $ok = 1;
  $for_cond$4 : while (1) {
   if ($ok) break $for_cond$4;
   var $inc = $ok + 1;
   if ($inc == 9999) break $while_body$2;
  }
  continue $while_body$2;
 }
 next();
 b$while_body$2 : while (1) {
  $ok = 1;
  b$for_cond$4 : while (1) {
   if ($ok) break b$for_cond$4;
   var $inc = $ok + 1;
  }
  continue b$while_body$2;
 }
 next();
 $once : do {
  if (!$ok) break $once;
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
 c$once : do {
  something();
 } while (0);
}
function bits() {
 print(($s & 65535) + ((($f & 65535) << 16 >> 16) * (($f & 65535) << 16 >> 16) | 0) % 256 & 65535);
 z(HEAP32[$id + 40 >> 2]);
 z($f << 2);
 z($f | 255);
 z($f & 255);
 z($f ^ 1);
 z($f << 2);
 z($f * 100 << 2);
 z(($f | 0) % 2 | 255);
 z(($f | 0) / 55 & 255);
 z($f - 22 ^ 1);
 z($f + 15 << 2);
}
function maths() {
 check(17);
 check(95);
 __ZN6b2Vec2C1Ev($this1 + 76 | 0);
 __ZN6b2Vec2C1Ev($this1 + 20 + 8 + 8 + 8 + 8 + 8 + 8 + 8 | 0);
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
 cheez : do {
  if ($i < $N) {
   if (callOther()) break cheez;
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
  label = 2;
 } else {
  somethingElse();
 }
 if (label == 55) {
  callOther();
 }
 pause(6);
 if ($i >= $N) {
  somethingElse();
 }
 pause(7);
 free : while (1) {
  if ($i >= $N) {
   label = 3;
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
 $if_then96$$if_end110thread_pre_split$48 : do {
  if ($cmp95) {
   if (!$cmp103) {
    label = 38;
    break $if_then96$$if_end110thread_pre_split$48;
   }
   if (!$cmp106) {
    label = 38;
    break $if_then96$$if_end110thread_pre_split$48;
   }
   label = 39;
   break $if_then96$$if_end110thread_pre_split$48;
  }
  label = 38;
 } while (0);
 $if_end110$$if_end110thread_pre_split$52 : do {
  if (label == 38) {
   var $79 = $_pr6;
  }
 } while (0);
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
 $if_then$$lor_lhs_false$2 : do {
  if (!$cmp) {
   if (something()) {
    label = 3;
    break $if_then$$lor_lhs_false$2;
   }
   more();
   break $if_then$$lor_lhs_false$2;
  }
  label = 3;
 } while (0);
 $if_then$$return$6 : do {
  if (label == 3) {
   final();
  }
 } while (0);
}
function lua() {
 $5$98 : while (1) {
  $15$$16$101 : do {
   if (!$14) {
    var $17 = $i;
    var $18 = $3;
    var $19 = $18 + ($17 << 2) | 0;
    var $20 = HEAP32[$19 >> 2];
    var $21 = $20 + 1 | 0;
    var $22 = HEAP8[$21];
    var $23 = $22 << 24 >> 24;
    break $15$$16$101;
   }
  } while (0);
 }
 pause();
 if ($1435 == 0) {
  label = 176;
  cheez();
 } else if ($1435 != 1) {
  label = 180;
  cheez();
 }
 pause();
 if ($1435 == 0) {
  label = 176;
  cheez();
 }
}
function moreLabels() {
 $for_cond$2 : while (1) {
  if (!$cmp) {
   break $for_cond$2;
  }
  $if_then$$for_inc$5 : do {
   if ($cmp1) {
    break $for_cond$2;
   }
   inc();
  } while (0);
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
 if (HEAP32[$incdec_ptr71_i + 8 >> 2]) {
  shoo();
 }
}
function tricky() {
 var $conv642 = $conv6374 - (($132 << 16 >> 16 | 0) / 2 & -1) & 65535;
}
function asmy() {
 f(HEAPU8[_buf + i6 & 16777215]);
 f(HEAPU8[_buf + i6 & 16777215]);
 f(HEAPU8[_buf + i6 & 16777215] & 1);
 f(HEAPU8[_buf + i6 & 16777215] & 1);
 f(HEAP8[_buf + i6 & 16777215] & 1);
 f(HEAPU8[_buf + i6 & 16777215] & 1);
 f((HEAPU8[_buf + i6 & 16777215] & 1) + i5 | 0);
 f((HEAPU8[_buf + i6 & 16777215] & 1) + i5 | 0);
 f((HEAP8[_buf + i6 & 16777215] & 1) + i5 | 0);
 f((HEAPU8[_buf + i6 & 16777215] & 1) + i5 | 0);
 f(HEAP32[12]);
 f(HEAPU32[12]);
 f(HEAP32[5]);
 f(HEAPU32[5]);
 if ((_sbrk($419 | 0) | 0) == -1) {
  print("fleefl");
 }
}
function dblMax() {
 var x = +1.7976931348623157e+308;
}
