function test() {
 var r1, r2;
 r1 = 0;
 f(r1);
 r1 += 1;
 r2 = r1 + 2;
 g(r1, r2);
 f(r1);
 r1 = cheez();
 r2 = r1 + 2;
 g(r2, r2);
 r2 = 200;
 r2 = 203;
 r2 = 205;
 r1 = 208;
 c(r2);
 while (f()) {
  r2 = 5;
  r1 = 12;
  gg(r2, r1 * 2);
  r1 = 100;
  gg(r1, 20);
 }
 r1 = f(), r2 = 100, r2 = 1e3, r2 = 1e5;
 f(r1());
}
function primes() {
 var r1, r2, r3, r4, r5, r6, r7;
 r1 = 2;
 r2 = 0;
 $_$2 : while (1) {
  r3 = r1 | 0;
  r4 = _sqrtf(r3);
  r3 = 2;
  $_$4 : while (1) {
   r5 = r3 | 0;
   r6 = r5 < r4;
   if (!r6) {
    r7 = 1;
    break $_$4;
   }
   r6 = (r1 | 0) % (r3 | 0);
   r5 = (r6 | 0) == 0;
   if (r5) {
    r7 = 0;
    break $_$4;
   }
   r5 = r3 + 1 | 0;
   r3 = r5;
  }
  r3 = r7 + r2 | 0;
  r4 = r1 + 1 | 0;
  r5 = (r3 | 0) < 1e5;
  if (r5) {
   r1 = r4;
   r2 = r3;
  } else {
   break $_$2;
  }
 }
 r2 = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r1, tempInt));
 return 1;
 return null;
}
function atomic() {
 var r1, r2, r3, r4;
 r1 = STACKTOP;
 STACKTOP += 4;
 r2 = r1 >> 2;
 HEAP32[r2] = 10;
 r3 = (tempValue = HEAP32[r2], HEAP32[r2] == 10 && (HEAP32[r2] = 7), tempValue);
 r4 = (r3 | 0) == 10 & 1;
 r3 = HEAP32[r2];
 r2 = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = r3, HEAP32[tempInt + 4 >> 2] = r4, tempInt));
 STACKTOP = r1;
 return 0;
 return null;
}
function fcntl_open() {
 var r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17;
 r1 = STACKTOP;
 STACKTOP += 84;
 r2 = r1;
 r3 = r1 + 72;
 r4 = r3 | 0;
 for (r5 = STRING_TABLE.__ZZ4mainE16nonexistent_name | 0, r6 = r4, r7 = r5 + 12; r5 < r7; r5++, r6++) {
  HEAP8[r6] = HEAP8[r5];
 }
 r5 = (r2 + 8 | 0) >> 2;
 r8 = r2 >> 2;
 r9 = r3 + 9 | 0;
 r10 = r3 + 10 | 0;
 r3 = 0;
 while (1) {
  r11 = HEAP32[__ZZ4mainE5modes + (r3 << 2) >> 2];
  r12 = r11 | 512;
  r13 = r3 + 97 & 255;
  r14 = 0;
  while (1) {
   r15 = (r14 & 1 | 0) == 0 ? r11 : r12;
   r16 = (r14 & 2 | 0) == 0 ? r15 : r15 | 2048;
   r15 = (r14 & 4 | 0) == 0 ? r16 : r16 | 1024;
   r16 = (r14 & 8 | 0) == 0 ? r15 : r15 | 8;
   r15 = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = r3, HEAP32[tempInt + 4 >> 2] = r14, tempInt));
   r15 = _open(STRING_TABLE.__str2 | 0, r16, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = 511, tempInt));
   r17 = (r15 | 0) != -1 & 1;
   r15 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r17, tempInt));
   r17 = ___errno();
   r15 = HEAP32[r17 >> 2];
   r17 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r15, tempInt));
   r15 = _stat(STRING_TABLE.__str2 | 0, r2);
   r15 = HEAP32[r5] & -512;
   r17 = _printf(STRING_TABLE.__str4 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r15, tempInt));
   for (r6 = r8, r7 = r6 + 18; r6 < r7; r6++) {
    HEAP32[r6] = 0;
   }
   r15 = _putchar(10);
   r15 = ___errno();
   HEAP32[r15 >> 2] = 0;
   r15 = _printf(STRING_TABLE.__str6 | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = r3, HEAP32[tempInt + 4 >> 2] = r14, tempInt));
   r15 = _open(STRING_TABLE.__str7 | 0, r16, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = 511, tempInt));
   r17 = (r15 | 0) != -1 & 1;
   r15 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r17, tempInt));
   r17 = ___errno();
   r15 = HEAP32[r17 >> 2];
   r17 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r15, tempInt));
   r15 = _stat(STRING_TABLE.__str7 | 0, r2);
   r15 = HEAP32[r5] & -512;
   r17 = _printf(STRING_TABLE.__str4 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r15, tempInt));
   for (r6 = r8, r7 = r6 + 18; r6 < r7; r6++) {
    HEAP32[r6] = 0;
   }
   r15 = _putchar(10);
   r15 = ___errno();
   HEAP32[r15 >> 2] = 0;
   HEAP8[r9] = r13;
   HEAP8[r10] = r14 + 97 & 255;
   r15 = _printf(STRING_TABLE.__str8 | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = r3, HEAP32[tempInt + 4 >> 2] = r14, tempInt));
   r15 = _open(r4, r16, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = 511, tempInt));
   r17 = (r15 | 0) != -1 & 1;
   r15 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r17, tempInt));
   r17 = ___errno();
   r15 = HEAP32[r17 >> 2];
   r17 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r15, tempInt));
   r15 = _stat(r4, r2);
   r15 = HEAP32[r5] & -512;
   r17 = _printf(STRING_TABLE.__str4 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r15, tempInt));
   for (r6 = r8, r7 = r6 + 18; r6 < r7; r6++) {
    HEAP32[r6] = 0;
   }
   r16 = _putchar(10);
   r16 = ___errno();
   HEAP32[r16 >> 2] = 0;
   r16 = r14 + 1 | 0;
   if ((r16 | 0) == 16) {
    break;
   }
   r14 = r16;
  }
  r14 = r3 + 1 | 0;
  if ((r14 | 0) == 3) {
   break;
  }
  r3 = r14;
 }
 r3 = _puts(STRING_TABLE._str | 0);
 r3 = _creat(STRING_TABLE.__str10 | 0, 511);
 r6 = (r3 | 0) != -1 & 1;
 r3 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r6, tempInt));
 r6 = ___errno();
 r3 = HEAP32[r6 >> 2];
 r6 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r3, tempInt));
 STACKTOP = r1;
 return 0;
 return null;
}
function ex() {
 var __stackBase__ = STACKTOP;
 STACKTOP += 4;
 var $e1 = __stackBase__;
 var $puts = _puts(STRING_TABLE._str17 | 0);
 var $x41 = $e1 | 0;
 var $i_04 = 0;
 while (1) {
  var $i_04;
  var $call1 = _printf(STRING_TABLE.__str15 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $i_04, tempInt));
  ((function() {
   try {
    __THREW__ = false;
    return __Z5magici($i_04);
   } catch (e) {
    if (typeof e != "number") throw e;
    if (ABORT) throw e;
    __THREW__ = true;
    return null;
   }
  }))();
 }
}
function switchey(r1) {
 var r2, r3, r4, r5, r6, r7, r8, r9;
 r2 = 5;
 while (1) {
  switch (r1 = f(r1, r2)) {
  case 1:
   g(r2);
   r3 = r1 + 1;
   r1--;
   break;
  case 2:
   g(r2 * 2);
   r4 = r1 + 22;
   r5 = r4 + 5;
   r1 -= 20;
   break;
  default:
   r6 = r1 + 22;
   r7 = r4 + 5;
   ch(r6, r7 * r4);
   throw 99;
  }
 }
 r8 = r1 + 1;
 p(r2, r8);
 r9 = r1 + 2;
 pp(r9);
}
function __ZN14NetworkAddressC1EPKcti(r1) {
 __ZN14NetworkAddressC2EPKcti(r1);
 return;
}

