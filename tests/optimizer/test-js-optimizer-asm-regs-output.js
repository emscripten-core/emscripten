function asm(d1, i2) {
 d1 = +d1;
 i2 = i2 | 0;
 var i3 = 0, d4 = +0;
 i2 = d1 + d1 | 0;
 d1 = d(Math_max(10, Math_min(5, f())));
 i3 = i2 + 2 | 0;
 print(i3);
 d4 = d1 * 5;
 return d4;
}
function _doit(i1, i2, i3) {
 i1 = i1 | 0;
 i2 = i2 | 0;
 i3 = i3 | 0;
 i1 = STACKTOP;
 _printf(__str | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[(tempInt & 16777215) >> 2] = i2, HEAP32[(tempInt + 4 & 16777215) >> 2] = i3, tempInt));
 STACKTOP = i1;
 return 0 | 0;
}
function stackRestore(i1) {
 i1 = i1 | 0;
 STACKTOP = i1;
}
function switchey(d1, i2) {
 d1 = +d1;
 i2 = i2 | 0;
 var i3 = 0, d4 = +0;
 switch (d1 | 0) {
 case 0:
  i2 = d1 + d1 | 0;
  d1 = d(Math_max(10, Math_min(5, f())));
  i3 = i2 + 2 | 0;
  print(i3);
  d4 = d1 * 5;
  return d4;
 case 1:
  return 20;
 }
}
function switchey2() {
 var i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, d6 = +0, d7 = +0, i8 = 0, i9 = 0;
 i1 = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 i2 = 1;
 while (1) switch (i2 | 0) {
 case 1:
  i3 = i1 | 0;
  __ZN6RandomC1Ev(i3);
  i4 = 0;
  i5 = 0;
  i2 = 2;
  break;
 case 2:
  d6 = +__ZN6Random3getEf(8, +1);
  d7 = +__ZN6Random3getEf(i3, +1);
  _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = d6, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = d7, tempInt) | 0);
  i8 = (d6 != d7 & 1) + i5 | 0;
  i9 = i4 + 1 | 0;
  if ((i9 | 0) < 100) {
   i4 = i9;
   i5 = i8;
   i2 = 2;
   break;
  } else {
   i2 = 3;
   break;
  }
 case 3:
  _printf(16, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[CHECK_ALIGN_4(tempInt | 0) >> 2] = i8, tempInt) | 0);
  STACKTOP = i1;
  return 0;
 }
 return 0;
}
function iffey() {
 var i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, d6 = +0, d7 = +0, i8 = 0, i9 = 0;
 i1 = STACKTOP;
 STACKTOP = STACKTOP + 8 | 0;
 i2 = 1;
 while (1) {
  if (i2 | 0) {
   i3 = i1 | 0;
   __ZN6RandomC1Ev(i3);
   i4 = 0;
   i5 = 0;
   i2 = 2;
  } else {
   d6 = +__ZN6Random3getEf(8, +1);
   d7 = +__ZN6Random3getEf(i3, +1);
   _printf(24, (tempInt = STACKTOP, STACKTOP = STACKTOP + 16 | 0, HEAPF64[CHECK_ALIGN_8(tempInt | 0) >> 3] = d6, HEAPF64[CHECK_ALIGN_8(tempInt + 8 | 0) >> 3] = d7, tempInt) | 0);
   i8 = (d6 != d7 & 1) + i5 | 0;
   i9 = i4 + 1 | 0;
   if ((i9 | 0) < 100) {
    i4 = i9;
    i5 = i8;
    i2 = 2;
   } else {
    i2 = 3;
    return 10;
   }
  }
 }
 return 0;
}
function nops() {
 var i1 = 0;
 f(i1);
 if (cheez) {
  doIt();
 } else {
  doIt();
 }
 if (cheez) {
  doIt();
 }
 if (!cheez) {
  doIt();
 }
 if (!cheez) {
  doIt();
 }
 doIt();
}

