function test() {
  var i = 0;
  f(i);
  i++;
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
  var __label__;
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
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test", "primes", "atomic"]
