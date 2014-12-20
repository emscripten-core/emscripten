function test() {
 var i = 0;
 f(i);
 i+=1;
 var j = i + 2;
 g(i, j);
 f(i);
 var i2 = cheez();
 var j2 = i2 + 2;
 g(j2, j2);
 var k1 = 200;
 var k2 = 203;
 var k3 = 205;
 var k4 = 208;
 c(k3);
 while (f()) {
  var apple = 5;
  var orange = 12;
  gg(apple, orange*2);
  var tangerine = 100;
  gg(tangerine, 20);
 }
 var ck = f(), ck2 = 100, ck3 = 1000, ck4 = 100000;
 f(ck());
}
function primes() {
 var label;
 var $curri_01 = 2;
 var $primes_02 = 0;
 $_$2 : while (1) {
  var $primes_02;
  var $curri_01;
  var $conv1 = $curri_01 | 0;
  var $call = _sqrtf($conv1);
  var $j_0 = 2;
  $_$4 : while (1) {
   var $j_0;
   var $conv = $j_0 | 0;
   var $cmp2 = $conv < $call;
   if (!$cmp2) {
    var $ok_0 = 1;
    break $_$4;
   }
   var $rem = ($curri_01 | 0) % ($j_0 | 0);
   var $cmp3 = ($rem | 0) == 0;
   if ($cmp3) {
    var $ok_0 = 0;
    break $_$4;
   }
   var $inc = $j_0 + 1 | 0;
   var $j_0 = $inc;
  }
  var $ok_0;
  var $inc5_primes_0 = $ok_0 + $primes_02 | 0;
  var $inc7 = $curri_01 + 1 | 0;
  var $cmp = ($inc5_primes_0 | 0) < 1e5;
  if ($cmp) {
   var $curri_01 = $inc7;
   var $primes_02 = $inc5_primes_0;
  } else {
   break $_$2;
  }
 }
 var $call8 = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $curri_01, tempInt));
 return 1;
 return null;
}
function atomic() {
 var $x$s2;
 var __stackBase__ = STACKTOP;
 STACKTOP += 4;
 var $x$s2 = __stackBase__ >> 2;
 HEAP32[$x$s2] = 10;
 var $0 = (tempValue = HEAP32[$x$s2], HEAP32[$x$s2] == 10 && (HEAP32[$x$s2] = 7), tempValue);
 var $conv = ($0 | 0) == 10 & 1;
 var $2 = HEAP32[$x$s2];
 var $call = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = $2, HEAP32[tempInt + 4 >> 2] = $conv, tempInt));
 STACKTOP = __stackBase__;
 return 0;
 return null;
}
function fcntl_open() {
 var $1$s2;
 var $st_mode$s2;
 var __stackBase__ = STACKTOP;
 STACKTOP += 84;
 var $s = __stackBase__;
 var $nonexistent_name = __stackBase__ + 72;
 var $0 = $nonexistent_name | 0;
 for (var $$src = STRING_TABLE.__ZZ4mainE16nonexistent_name | 0, $$dest = $0, $$stop = $$src + 12; $$src < $$stop; $$src++, $$dest++) {
  HEAP8[$$dest] = HEAP8[$$src];
 }
 var $st_mode$s2 = ($s + 8 | 0) >> 2;
 var $1$s2 = $s >> 2; // critical variable, becomes r8
 var $arrayidx43 = $nonexistent_name + 9 | 0;
 var $arrayidx46 = $nonexistent_name + 10 | 0;
 var $i_04 = 0;
 while (1) {
  var $i_04;
  var $2 = HEAP32[__ZZ4mainE5modes + ($i_04 << 2) >> 2];
  var $or = $2 | 512;
  var $conv42 = $i_04 + 97 & 255;
  var $j_03 = 0;
  while (1) {
   var $j_03;
   var $flags_0 = ($j_03 & 1 | 0) == 0 ? $2 : $or;
   var $flags_0_or7 = ($j_03 & 2 | 0) == 0 ? $flags_0 : $flags_0 | 2048;
   var $flags_2 = ($j_03 & 4 | 0) == 0 ? $flags_0_or7 : $flags_0_or7 | 1024;
   var $flags_2_or17 = ($j_03 & 8 | 0) == 0 ? $flags_2 : $flags_2 | 8;
   var $call = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = $i_04, HEAP32[tempInt + 4 >> 2] = $j_03, tempInt));
   var $call19 = _open(STRING_TABLE.__str2 | 0, $flags_2_or17, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = 511, tempInt));
   var $conv = ($call19 | 0) != -1 & 1;
   var $call21 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $conv, tempInt));
   var $call22 = ___errno();
   var $3 = HEAP32[$call22 >> 2];
   var $call23 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $3, tempInt));
   var $call24 = _stat(STRING_TABLE.__str2 | 0, $s);
   var $and25 = HEAP32[$st_mode$s2] & -512;
   var $call26 = _printf(STRING_TABLE.__str4 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $and25, tempInt));
   for (var $$dest = $1$s2, $$stop = $$dest + 18; $$dest < $$stop; $$dest++) {
    HEAP32[$$dest] = 0;
   }
   var $putchar = _putchar(10);
   var $call28 = ___errno();
   HEAP32[$call28 >> 2] = 0;
   var $call29 = _printf(STRING_TABLE.__str6 | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = $i_04, HEAP32[tempInt + 4 >> 2] = $j_03, tempInt));
   var $call30 = _open(STRING_TABLE.__str7 | 0, $flags_2_or17, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = 511, tempInt));
   var $conv32 = ($call30 | 0) != -1 & 1;
   var $call33 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $conv32, tempInt));
   var $call34 = ___errno();
   var $5 = HEAP32[$call34 >> 2];
   var $call35 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $5, tempInt));
   var $call36 = _stat(STRING_TABLE.__str7 | 0, $s);
   var $and38 = HEAP32[$st_mode$s2] & -512;
   var $call39 = _printf(STRING_TABLE.__str4 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $and38, tempInt));
   for (var $$dest = $1$s2, $$stop = $$dest + 18; $$dest < $$stop; $$dest++) {
    HEAP32[$$dest] = 0;
   }
   var $putchar1 = _putchar(10);
   var $call41 = ___errno();
   HEAP32[$call41 >> 2] = 0;
   HEAP8[$arrayidx43] = $conv42;
   HEAP8[$arrayidx46] = $j_03 + 97 & 255;
   var $call47 = _printf(STRING_TABLE.__str8 | 0, (tempInt = STACKTOP, STACKTOP += 8, HEAP32[tempInt >> 2] = $i_04, HEAP32[tempInt + 4 >> 2] = $j_03, tempInt));
   var $call48 = _open($0, $flags_2_or17, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = 511, tempInt));
   var $conv50 = ($call48 | 0) != -1 & 1;
   var $call51 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $conv50, tempInt));
   var $call52 = ___errno();
   var $7 = HEAP32[$call52 >> 2];
   var $call53 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $7, tempInt));
   var $call55 = _stat($0, $s);
   var $and57 = HEAP32[$st_mode$s2] & -512;
   var $call58 = _printf(STRING_TABLE.__str4 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $and57, tempInt));
   for (var $$dest = $1$s2, $$stop = $$dest + 18; $$dest < $$stop; $$dest++) {
    HEAP32[$$dest] = 0;
   }
   var $putchar2 = _putchar(10);
   var $call60 = ___errno();
   HEAP32[$call60 >> 2] = 0;
   var $inc = $j_03 + 1 | 0;
   if (($inc | 0) == 16) {
    break;
   }
   var $j_03 = $inc;
  }
  var $inc62 = $i_04 + 1 | 0;
  if (($inc62 | 0) == 3) {
   break;
  }
  var $i_04 = $inc62;
 }
 var $puts = _puts(STRING_TABLE._str | 0);
 var $call65 = _creat(STRING_TABLE.__str10 | 0, 511);
 var $conv67 = ($call65 | 0) != -1 & 1;
 var $call68 = _printf(STRING_TABLE.__str1 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $conv67, tempInt));
 var $call69 = ___errno();
 var $9 = HEAP32[$call69 >> 2];
 var $call70 = _printf(STRING_TABLE.__str3 | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = $9, tempInt));
 STACKTOP = __stackBase__;
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
  ((function() { // prevents registerize, looks like inline asm
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
function switchey(x) {
 var a = 5;
 while (1) {
  switch (x = f(x, a)) {
   case 1:
    g(a);
    var b = x+1;
    x--;
    break;
   case 2:
    g(a*2);
    var c = x+22;
    var d = c+5;
    x -= 20;
    break;
   default:
    var c1 = x+22;
    var d2 = c+5;
    ch(c1, d2*c);
    throw 99;
  }
 }
 var aa = x+1;
 p(a, aa);
 var aaa = x+2;
 pp(aaa);
}
function __ZN14NetworkAddressC1EPKcti($this) {
 __ZN14NetworkAddressC2EPKcti($this);
 return;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test", "primes", "atomic", "fcntl_open", "ex", "switchey", "__ZN14NetworkAddressC1EPKcti"]
