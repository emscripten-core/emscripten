function test() {
  var r1, r2, r3;
  r1 = 0;
  f(r1);
  r1++;
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
    r3 = 100;
    gg(r3, 20);
  }
  r3 = f(), r1 = 100, r1 = 1e3, r1 = 1e5;
  f(r3());
}
function primes() {
  var r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14;
  r1 = 2;
  r2 = 0;
  $_$2 : while (1) {
    r3 = r1 | 0;
    r4 = _sqrtf(r3);
    r5 = 2;
    $_$4 : while (1) {
      r6 = r5 | 0;
      r7 = r6 < r4;
      if (!r7) {
        r8 = 1;
        break $_$4;
      }
      r9 = (r1 | 0) % (r5 | 0);
      r10 = (r9 | 0) == 0;
      if (r10) {
        r8 = 0;
        break $_$4;
      }
      r11 = r5 + 1 | 0;
      r5 = r11;
    }
    r12 = r8 + r2 | 0;
    r13 = r1 + 1 | 0;
    r14 = (r12 | 0) < 1e5;
    if (r14) {
      r1 = r13;
      r2 = r12;
    } else {
      break $_$2;
    }
  }
  r12 = _printf(STRING_TABLE.__str | 0, (tempInt = STACKTOP, STACKTOP += 4, HEAP32[tempInt >> 2] = r1, tempInt));
  return 1;
  return null;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test", "primes"]
