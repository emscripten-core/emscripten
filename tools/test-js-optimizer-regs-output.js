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
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test", "primes", "atomic"]
