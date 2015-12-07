function asm(d1, i2) {
 d1 = +d1;
 i2 = i2 | 0;
 i2 = d1 + d1 | 0;
 d1 = d(Math_max(10, Math_min(5, f())));
 i2 = i2 + 2 | 0;
 print(i2);
 d1 = d1 * 5;
 return d1;
}
function _doit(i3, i2, i1) {
 i3 = i3 | 0;
 i2 = i2 | 0;
 i1 = i1 | 0;
 i3 = STACKTOP;
 _printf(__str | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[(tempInt & 16777215) >> 2] = i2, HEAP32[(tempInt + 4 & 16777215) >> 2] = i1, tempInt));
 STACKTOP = i3;
 return 0 | 0;
}
function stackRestore(i1) {
 i1 = i1 | 0;
 STACKTOP = i1;
}
function switchey(d1, i2) {
 d1 = +d1;
 i2 = i2 | 0;
 switch (d1 | 0) {
 case 0:
  i2 = d1 + d1 | 0;
  d1 = d(Math_max(10, Math_min(5, f())));
  i2 = i2 + 2 | 0;
  print(i2);
  d1 = d1 * 5;
  return d1;
 case 1:
  return 20;
 }
}
function switchey2() {
 var i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, d6 = +0, d7 = +0;
 i2 = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 i3 = 1;
 while (1) switch (i3 | 0) {
 case 1:
  i1 = i2 | 0;
  __ZN6RandomC1Ev(i1);
  i4 = 0;
  i5 = 0;
  i3 = 2;
  break;
 case 2:
  d7 = +__ZN6Random3getEf(8, +1);
  d6 = +__ZN6Random3getEf(i1, +1);
  _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = d7, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = d6, tempInt) | 0);
  i5 = (d7 != d6 & 1) + i5 | 0;
  i4 = i4 + 1 | 0;
  if ((i4 | 0) < 100) {
   i3 = 2;
   break;
  } else {
   i3 = 3;
   break;
  }
 case 3:
  _printf(16, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[CHECK_ALIGN_4(tempInt | 0) >> 2] = i5, tempInt) | 0);
  STACKTOP = i2;
  return 0;
 }
 return 0;
}
function iffey() {
 var i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, d6 = +0, d7 = +0;
 i2 = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 i4 = 1;
 while (1) {
  if (i4 | 0) {
   i1 = i2 | 0;
   __ZN6RandomC1Ev(i1);
   i3 = 0;
   i5 = 0;
   i4 = 2;
  } else {
   d7 = +__ZN6Random3getEf(8, +1);
   d6 = +__ZN6Random3getEf(i1, +1);
   _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = d7, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = d6, tempInt) | 0);
   i5 = (d7 != d6 & 1) + i5 | 0;
   i3 = i3 + 1 | 0;
   if ((i3 | 0) < 100) {
    i4 = 2;
   } else {
    return 10;
   }
  }
 }
 return 0;
}
function labelledJump(i3) {
 i3 = i3 | 0;
 var i1 = 0, i2 = 0;
 i2 = 2;
 if (i3) {
  i2 = 17;
  i1 = 1;
 }
 if (i1 == 1) {
  i3 = i2 + 1;
 } else {
  i3 = i2 + 1;
 }
 return i3;
}
function linkedVars() {
 var i1 = 0, i2 = 0;
 while (1) {
  i2 = 9;
  i1 = 5;
  while (i2 > 0 | i1 > 0) {
   if (i2 < i1) {
    i2 = i2 - 1;
   } else {
    i1 = i1 - 1;
   }
  }
  if (i2 < i1) {
   break;
  }
 }
 return i2 + i1;
}
function deadCondExpr(i2) {
 i2 = i2 | 0;
 var i1 = 0;
 return i1 | 0;
}
function b1() {
 abort(1);
 return SIMD_Float32x4(0, 0, 0, 0);
}

