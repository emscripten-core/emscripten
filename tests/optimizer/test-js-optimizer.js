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
function bits() {
 print((($s & 65535) + ((($f & 65535) << 16 >> 16) * (($f & 65535) << 16 >> 16) | 0 | 0) % 256 | 0) & 65535);
 z(HEAP32[($id + 40 | 0) >> 2]);
 z(($f | 0) << 2);
 z(($f | 0) | 255);
 z(($f | 0) & 255);
 z(($f | 0) ^ 1);
 z(($f | 0) << 2);
 z((($f | 0) * 100) << 2);
 z((($f | 0) % 2) | 255);
 z((($f | 0) / 55) & 255);
 z((($f | 0) - 22) ^ 1);
 z((($f | 0) + 15) << 2);
}
function maths() {
 check(5+12);
 check(90+3+2);
 __ZN6b2Vec2C1Ev($this1 + ((((((((20 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) | 0);
 __ZN6b2Vec2C1Ev(((((((($this1 + 20 | 0 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0) + 8 | 0);
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
   while(true) { break $for_body3$$for_end$5 }
   callOther();
  }
 } while (0);
 pause(2);
 if ($i < $N) {
  label = 2;
 } else {
  label = 3;
 }
 cheez: do {
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
 free: while (1) {
  if ($i < $N) {
   label = 2;
  } else {
   label = 3; // this cannot be removed!
   break;
  }
  if (label == 2) {
   somethingElse();
  }
  if ($i < $N) {
   label = 2;
  } else {
   label = 3; // this can be removed!
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
 while(check()) {
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
function innerShouldAlsoBeHoisted() {
 function hoisting() {
  if ($i < $N) {
   label = 2;
  }
  if (label == 2) {
   callOther();
  }
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
  label = 3;
 } else {
  label = 1;
 }
 $if_then$$lor_lhs_false$2 : do {
  if (label == 1) {
   if (something()) {
    label = 3;
    break $if_then$$lor_lhs_false$2;
   }
   more();
   break $if_then$$lor_lhs_false$2;
  }
 } while (0);
 $if_then$$return$6 : do {
  if (label == 3) {
   final();
  }
 } while (0);
}
function lua() {
 $5$98 : while (1) {
  if ($14) {
   label = 3;
  } else {
   label = 4;
  }
  $15$$16$101 : do {
   if (label == 3) {} else if (label == 4) {
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
  label = 176;
  cheez();
 } else if ($1435 == 1) {} else {
  label = 180;
  cheez();
 }
 pause();
 if ($1435 == 0) {
  label = 176;
  cheez();
 } else if ($1435 == 1) {}
}
function moreLabels() {
 $for_cond$2 : while (1) { // even this label should vanish
  if (!$cmp) {
   break $for_cond$2;
  }
  $if_then$$for_inc$5 : do {
   if ($cmp1) {
    break $for_cond$2;
   } else {
    inc();
   }
  } while (0);
  $if_then$$for_inc$5 : do {} while (0);
 }
 pause(999);
 $while_body$$while_end$31 : do {
  if ($cmp3) {
   var $6 = $5;
   {
    while (1) {
     var $6;
     $iter = $6 + 3;
     if (FHEAP[$iter + 1] < $pct_addr) {
      var $6 = $iter;
     } else {
      var $_lcssa = $iter;
      break $while_body$$while_end$31;
     }
    }
   }
  } else {
   var $_lcssa = $5;
   label = 2;
  }
 } while (0);
 var $_lcssa;
 cheez();
}
function notComps() {
 if (!(HEAP32[$incdec_ptr71_i + 8 >> 2] == 0)) {
  shoo();
 }
}
function tricky() {
 // The &-1 is a rounding correction, and must not be removed
 var $conv642 = ($conv6374 - (($132 << 16 >> 16 | 0) / 2 & -1) | 0) & 65535;
}
function asmy() {
 f(HEAP8[_buf + i6 & 16777215] & 255);
 f(HEAPU8[_buf + i6 & 16777215] & 255);
 f(HEAP8[_buf + i6 & 16777215] & 255 & 1);
 f(HEAPU8[_buf + i6 & 16777215] & 255 & 1);
 f(HEAP8[_buf + i6 & 16777215] & 1 & 255);
 f(HEAPU8[_buf + i6 & 16777215] & 1 & 255);
 f((HEAP8[_buf + i6 & 16777215] & 255 & 1) + i5 | 0);
 f((HEAPU8[_buf + i6 & 16777215] & 255 & 1) + i5 | 0);
 f((HEAP8[_buf + i6 & 16777215] & 1 & 255) + i5 | 0);
 f((HEAPU8[_buf + i6 & 16777215] & 1 & 255) + i5 | 0);
 f(HEAP32[((46 + 2)|0) >> 2]);
 f(HEAPU32[((46 + 2)|0) >> 2]);
 f(HEAP32[((((((43 + 3)|0) + 2)|0) >> 2) + 8) >> 2]);
 f(HEAPU32[((((((43 + 3)|0) + 2)|0) >> 2) + 8) >> 2]);
 if ((_sbrk($419 | 0) | 0 | 0) == -1) {
  print('fleefl');
 }
}
function dblMax() {
 var x = +1.7976931348623157E+308;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["abc", "xyz", "xyz2", "expr", "loopy", "bits", "maths", "hoisting", "demangle", "lua", "moreLabels", "notComps", "tricky", "asmy"]
