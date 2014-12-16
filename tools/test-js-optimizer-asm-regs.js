function asm(x, y) {
 x = +x;
 y = y | 0;
 var int1 = 0, int2 = 0; // do not mix the types!
 var double1 = +0, double2 = +0;
 int1 = (x+x)|0;
 double1 = d(Math_max(10, Math_min(5, f())));
 int2 = (int1+2)|0;
 print(int2);
 double2 = double1*5;
 return double2;
}
function _doit($x, $y$0, $y$1) {
 $x = $x | 0;
 $y$0 = $y$0 | 0;
 $y$1 = $y$1 | 0;
 var __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 _printf(__str | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[(tempInt & 16777215) >> 2] = $y$0, HEAP32[(tempInt + 4 & 16777215) >> 2] = $y$1, tempInt));
 STACKTOP = __stackBase__;
 return 0 | 0;
}
function stackRestore(top) {
 top = top|0;
 STACKTOP = top;
}
function switchey(x, y) {
 x = +x;
 y = y | 0;
 var int1 = 0, int2 = 0; // do not mix the types!
 var double1 = +0, double2 = +0;
 switch(x|0) {
  case 0:
   int1 = (x+x)|0;
   double1 = d(Math_max(10, Math_min(5, f())));
   int2 = (int1+2)|0;
   print(int2);
   double2 = double1*5;
   return double2;
  case 1:
   return 20;
 }
}
function switchey2() {
 var $rng2 = 0, $count_06 = 0, $i_05 = 0, $2 = +0, $3 = +0, $count_1 = 0, $9 = 0, label = 0, __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 label = 1;
 while (1) switch (label | 0) {
  case 1:
  $rng2 = __stackBase__ | 0;
  __ZN6RandomC1Ev($rng2);
  $i_05 = 0;
  $count_06 = 0;
  label = 2;
  break;
  case 2:
  $2 = +__ZN6Random3getEf(8, +1);
  $3 = +__ZN6Random3getEf($rng2, +1);
  _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = $2, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = $3, tempInt) | 0);
  $count_1 = ($2 != $3 & 1) + $count_06 | 0;
  $9 = $i_05 + 1 | 0;
  if (($9 | 0) < 100) {
   $i_05 = $9;
   $count_06 = $count_1;
   label = 2;
   break;
  } else {
   label = 3;
   break;
  }
  case 3:
  _printf(16, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[CHECK_ALIGN_4(tempInt | 0) >> 2] = $count_1, tempInt) | 0);
  STACKTOP = __stackBase__;
  return 0;
 }
 return 0;
}
function iffey() {
 var $rng2 = 0, $count_06 = 0, $i_05 = 0, $2 = +0, $3 = +0, $count_1 = 0, $9 = 0, label = 0, __stackBase__ = 0;
 __stackBase__ = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 label = 1;
 while (1) {
  if (label | 0) {
   $rng2 = __stackBase__ | 0;
   __ZN6RandomC1Ev($rng2);
   $i_05 = 0;
   $count_06 = 0;
   label = 2;
  } else {
   $2 = +__ZN6Random3getEf(8, +1);
   $3 = +__ZN6Random3getEf($rng2, +1);
   _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = $2, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = $3, tempInt) | 0);
   $count_1 = ($2 != $3 & 1) + $count_06 | 0;
   $9 = $i_05 + 1 | 0;
   if (($9 | 0) < 100) {
    $i_05 = $9;
    $count_06 = $count_1;
    label = 2;
   } else {
    label = 3;
    return 10;
   }
  }
 }
 return 0;
}
function nops() {
  var x = 0;
  x | 0;
  ~x;
  f(x);
  // vaccuming
  if (cheez) {
   doIt();
  } else {
   doIt();
  }
  if (cheez) {
   doIt();
  } else {}
  if (cheez) {} else {
   doIt();
  }
  if (cheez) {} else {
   doIt();
  }
  if (cheez) {} else {}
  if (doIt()) {} else {}
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "_doit", "stackRestore", "switchey", "switchey2", "iffey", "nops"]

