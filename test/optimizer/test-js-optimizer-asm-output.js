function expr() {
 if (!($0 < $1)) print("hi");
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
 z(($f | 0) * 100 << 2);
 z(($f | 0) % 2 | 255);
 z(($f | 0) / 55 & 255);
 z($f - 22 ^ 1);
 z($f + 15 << 2);
}
function maths() {
 check(17);
 check(95);
 __ZN6b2Vec2C1Ev($this1 + 20 + 8 + 8 + 8 + 8 + 8 + 8 + 8 | 0);
}
function hoisting() {
 if ($i < $N) {
  label = 2;
 }
 if (label == 2) {
  callOther();
 }
 pause(1);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 $for_body3$$for_end$5 : do {
  if (label == 2) {
   callOther();
  }
 } while (0);
 pause(2);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 cheez : do {
  if (label == 2) {
   if (callOther()) break cheez;
  }
 } while (0);
 pause(3);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 if (label == 2) {
  callOther();
 }
 pause(4);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 if (label == 2) {
  callOther();
 } else if (label == 3) {
  somethingElse();
 }
 pause(5);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 if (label == 55) {
  callOther();
 } else if (label == 3) {
  somethingElse();
 }
 pause(6);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 if (label == 3) {
  somethingElse();
 }
 pause(7);
 free : while (1) {
  if ($i < $N) {
   label = 2;
  } else {
   label = 3;
   break;
  }
  if (label == 2) {
   somethingElse();
  }
  if ($i < $N) {
   label = 2;
  } else {
   label = 3;
  }
  if (label == 2) {
   somethingElse();
  }
  nothing();
 }
 pause(8);
 var $cmp95 = $69 == -1;
 if ($cmp95) {
  label = 35;
 } else {
  label = 38;
 }
 $if_then96$$if_end110thread_pre_split$48 : do {
  if (label == 35) {
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
 } while (0);
 $if_end110$$if_end110thread_pre_split$52 : do {
  if (label == 38) {
   var $79 = $_pr6;
  }
 } while (0);
 pause(9);
 var $cmp70 = ($call69 | 0) != 0;
 if ($cmp70) {
  label = 40;
 } else {
  label = 41;
 }
 $if_then72$$if_end73$126 : do {
  if (label == 40) {} else if (label == 41) {}
 } while (0);
 pause(10);
 while (check()) {
  if ($i < $N) {
   label = 2;
  } else {
   label = 3;
  }
  if (label == 2) {
   callOther();
   break;
  } else if (label == 3) {
   somethingElse();
  }
  if ($i1 < $N) {
   label = 2;
  } else {
   label = 3;
  }
  if (label == 2) {
   callOther();
   continue;
  } else if (label == 3) {
   somethingElse();
  }
  if ($i2 < $N) {
   label = 2;
  } else {
   label = 3;
  }
  if (label == 2) {
   callOther();
  } else if (label == 3) {
   somethingElse();
   break;
  }
  if ($i3 < $N) {
   label = 2;
  } else {
   label = 3;
  }
  if (label == 2) {
   callOther();
  } else if (label == 3) {
   somethingElse();
   continue;
  }
  if ($i4 < $N) {
   label = 2;
  } else {
   label = 3;
  }
  if (label == 2) {
   callOther();
   break;
  } else if (label == 3) {
   somethingElse();
   continue;
  }
 }
}
function toZero(x, y) {
 x = x | 0;
 y = +y;
 if (x) a();
 if (y) a();
 while (x) a();
 while (y) a();
 do {
  a();
 } while (x);
 do {
  a();
 } while (y);
 x ? a() : b();
 y ? a() : b();
}

