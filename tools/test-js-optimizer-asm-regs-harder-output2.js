function asm(d2, i1) {
 d2 = +d2;
 i1 = i1 | 0;
 i1 = d2 + d2 | 0;
 d2 = d(Math_max(10, Math_min(5, f())));
 i1 = i1 + 2 | 0;
 print(i1);
 d2 = d2 * 5;
 return d2;
}

function _doit(i2, i3, i1) {
 i2 = i2 | 0;
 i3 = i3 | 0;
 i1 = i1 | 0;
 var i4 = 0;
 i4 = STACKTOP;
 _printf(__str | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[(tempInt & 16777215) >> 2] = i3, HEAP32[(tempInt + 4 & 16777215) >> 2] = i1, tempInt));
 STACKTOP = i4;
 return 0 | 0;
}

function stackRestore(i1) {
 i1 = i1 | 0;
 STACKTOP = i1;
}

function switchey(d2, i1) {
 d2 = +d2;
 i1 = i1 | 0;
 switch (d2 | 0) {
 case 0:
  i1 = d2 + d2 | 0;
  d2 = d(Math_max(10, Math_min(5, f())));
  i1 = i1 + 2 | 0;
  print(i1);
  d2 = d2 * 5;
  return d2;
 case 1:
  return 20;
 }
}

function switchey2() {
 var i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, d6 = +0, d7 = +0;
 i4 = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 i3 = 1;
 while (1) switch (i3 | 0) {
 case 1:
  i5 = i4 | 0;
  __ZN6RandomC1Ev(i5);
  i1 = 0;
  i2 = 0;
  i3 = 2;
  break;
 case 2:
  d7 = +__ZN6Random3getEf(8, +1);
  d6 = +__ZN6Random3getEf(i5, +1);
  _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = d7, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = d6, tempInt) | 0);
  i2 = (d7 != d6 & 1) + i2 | 0;
  i1 = i1 + 1 | 0;
  if ((i1 | 0) < 100) {
   i3 = 2;
   break;
  } else {
   i3 = 3;
   break;
  }
 case 3:
  _printf(16, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[CHECK_ALIGN_4(tempInt | 0) >> 2] = i2, tempInt) | 0);
  STACKTOP = i4;
  return 0;
 }
 return 0;
}

function iffey() {
 var i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, d6 = +0, d7 = +0;
 i4 = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 i3 = 1;
 while (1) {
  if (i3 | 0) {
   i5 = i4 | 0;
   __ZN6RandomC1Ev(i5);
   i1 = 0;
   i2 = 0;
   i3 = 2;
  } else {
   d7 = +__ZN6Random3getEf(8, +1);
   d6 = +__ZN6Random3getEf(i5, +1);
   _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = d7, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = d6, tempInt) | 0);
   i2 = (d7 != d6 & 1) + i2 | 0;
   i1 = i1 + 1 | 0;
   if ((i1 | 0) < 100) {
    i3 = 2;
   } else {
    return 10;
   }
  }
 }
 return 0;
}

function labelledJump(i2) {
 i2 = i2 | 0;
 var i1 = 0, i3 = 0;
 i1 = 2;
 if (i2) {
  i1 = 17;
  i3 = 1;
 }
 if (i3 == 1) {
  i2 = i1 + 1;
 } else {
  i2 = i1 + 1;
 }
 return i2;
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

function deadCondExpr(i1) {
 i1 = i1 | 0;
 var i2 = 0;
 return i2 | 0;
}

