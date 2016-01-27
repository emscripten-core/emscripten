var Module = {};
Module["asm"] = (function(global, env, buffer) {
 "use asm";
 var HEAP8 = new global.Int8Array(buffer);
 var HEAP32 = new global.Int32Array(buffer);
 var HEAPF64 = new global.Float64Array(buffer);
 var STACKTOP = env.STACKTOP | 0;
 var _atoi = env._atoi;
 var _strlen = env._strlen;
 var __Znaj = env.__Znaj;
 var _puts = env._puts;
 var _memcpy = env._memcpy;
 var __ZdaPv = env.__ZdaPv;
 var __ZdlPv = env.__ZdlPv;
 function _main(i1, i2) {
  i1 = i1 | 0;
  i2 = i2 | 0;
  var d3 = 0, i4 = 0, i5 = 0, i6 = 0, i7 = 0, i8 = 0, i9 = 0, i10 = 0, i11 = 0, i12 = 0, d13 = 0;
  i12 = STACKTOP;
  STACKTOP = STACKTOP + 4240 | 0;
  i9 = i12 + 2120 | 0;
  i11 = i12;
  if ((i1 | 0) > 1) i10 = _atoi(HEAP32[i2 + 4 >> 2] | 0) | 0; else i10 = 512;
  i8 = __Znaj(347) | 0;
  _memcpy(i8 | 0, 520, 287) | 0;
  i1 = i8 + 287 | 0;
  i2 = 520;
  i4 = i1 + 60 | 0;
  do {
   HEAP8[i1 >> 0] = HEAP8[i2 >> 0] | 0;
   i1 = i1 + 1 | 0;
   i2 = i2 + 1 | 0;
  } while ((i1 | 0) < (i4 | 0));
  i1 = i10 << 1;
  if (i1) {
   i2 = 0;
   while (1) {
    i7 = i1 >>> 0 < 60 ? i1 : 60;
    i4 = __Znaj(i7 + 2 | 0) | 0;
    _memcpy(i4 | 0, i8 + i2 | 0, i7 | 0) | 0;
    HEAP8[i4 + i7 >> 0] = 0;
    i5 = _strlen(i4 | 0) | 0;
    i6 = HEAP32[128] | 0;
    if ((i5 | 0) > (i6 | 0)) {
     if ((i6 | 0) > 0) {
      HEAP8[i4 + i6 >> 0] = 0;
      _puts(i4 | 0) | 0;
      HEAP32[128] = 0;
     }
    } else {
     _puts(i4 | 0) | 0;
     HEAP32[128] = (HEAP32[128] | 0) - i5;
    }
    __ZdlPv(i4 | 0);
    i2 = i7 + i2 | 0;
    if ((i1 | 0) == (i7 | 0)) break; else {
     i1 = i1 - i7 | 0;
     i2 = i2 >>> 0 > 287 ? i2 + -287 | 0 : i2;
    }
   }
  }
  __ZdaPv(i8 | 0);
  if (!(HEAP32[2] | 0)) {
   i1 = 8;
   i2 = 0;
  } else {
   i1 = 8;
   d3 = 0;
   do {
    i8 = i1 + 8 | 0;
    d3 = d3 + +HEAPF64[i8 >> 3];
    d13 = d3 < 1 ? d3 : 1;
    HEAPF64[i8 >> 3] = d13;
    HEAP32[i1 + 16 >> 2] = ~~(d13 * 512) >>> 0;
    i1 = i1 + 24 | 0;
   } while ((HEAP32[i1 >> 2] | 0) != 0);
   i1 = 8;
   i2 = 0;
  }
  do {
   while (1) {
    i8 = HEAP32[i1 + 16 >> 2] | 0;
    if (i2 >>> 0 > i8 >>> 0 & (i8 | 0) != 0) i1 = i1 + 24 | 0; else break;
   }
   HEAP32[i9 + (i2 << 2) >> 2] = i1;
   i2 = i2 + 1 | 0;
  } while ((i2 | 0) != 513);
  HEAP32[i9 + 2116 >> 2] = 0;
  i1 = i10 * 3 | 0;
  if (i1) while (1) {
   i2 = i1 >>> 0 < 60 ? i1 : 60;
   __ZN10Randomized9writelineEj(i9, i2);
   if ((i1 | 0) == (i2 | 0)) break; else i1 = i1 - i2 | 0;
  }
  if (!(HEAP32[98] | 0)) {
   i1 = 392;
   i2 = 0;
  } else {
   i1 = 392;
   d3 = 0;
   do {
    i9 = i1 + 8 | 0;
    d3 = d3 + +HEAPF64[i9 >> 3];
    d13 = d3 < 1 ? d3 : 1;
    HEAPF64[i9 >> 3] = d13;
    HEAP32[i1 + 16 >> 2] = ~~(d13 * 512) >>> 0;
    i1 = i1 + 24 | 0;
   } while ((HEAP32[i1 >> 2] | 0) != 0);
   i1 = 392;
   i2 = 0;
  }
  do {
   while (1) {
    i9 = HEAP32[i1 + 16 >> 2] | 0;
    if (i2 >>> 0 > i9 >>> 0 & (i9 | 0) != 0) i1 = i1 + 24 | 0; else break;
   }
   HEAP32[i11 + (i2 << 2) >> 2] = i1;
   i2 = i2 + 1 | 0;
  } while ((i2 | 0) != 513);
  HEAP32[i11 + 2116 >> 2] = 0;
  i1 = i10 * 5 | 0;
  if (!i1) {
   STACKTOP = i12;
   return 0;
  }
  while (1) {
   i2 = i1 >>> 0 < 60 ? i1 : 60;
   __ZN10Randomized9writelineEj(i11, i2);
   if ((i1 | 0) == (i2 | 0)) break; else i1 = i1 - i2 | 0;
  }
  STACKTOP = i12;
  return 0;
 }
 function __ZN10Randomized9writelineEj(i5, i2) {
  i5 = i5 | 0;
  i2 = i2 | 0;
  var i1 = 0, i3 = 0, d4 = 0, d6 = 0;
  if (i2) {
   i3 = 0;
   do {
    i1 = ((((HEAP32[129] | 0) * 3877 | 0) + 29573 | 0) >>> 0) % 139968 | 0;
    HEAP32[129] = i1;
    d6 = +(i1 >>> 0) / 139968;
    d4 = d6;
    i1 = HEAP32[i5 + (~~(d6 * 512) >>> 0 << 2) >> 2] | 0;
    while (1) if (+HEAPF64[i1 + 8 >> 3] < d4) i1 = i1 + 24 | 0; else break;
    HEAP8[i5 + 2052 + i3 >> 0] = HEAP32[i1 >> 2];
    i3 = i3 + 1 | 0;
   } while ((i3 | 0) != (i2 | 0));
  }
  HEAP8[i5 + 2052 + i2 >> 0] = 10;
  i1 = i2 + 1 | 0;
  HEAP8[i5 + 2052 + i1 >> 0] = 0;
  HEAP32[i5 + 2116 >> 2] = i1;
  i1 = i5 + 2052 | 0;
  i2 = _strlen(i1 | 0) | 0;
  i3 = HEAP32[128] | 0;
  if ((i2 | 0) <= (i3 | 0)) {
   _puts(i1 | 0) | 0;
   HEAP32[128] = (HEAP32[128] | 0) - i2;
   return;
  }
  if ((i3 | 0) <= 0) return;
  HEAP8[i5 + 2052 + i3 >> 0] = 0;
  _puts(i1 | 0) | 0;
  HEAP8[(HEAP32[128] | 0) + (i5 + 2052) >> 0] = 122;
  HEAP32[128] = 0;
  return;
 }
 return {
  _main: _main
 };
});

