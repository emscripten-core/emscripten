function finall(x) {
 x = +x;
 var a = +5;
 a = +x;
 a = 17;
 a = +44;
 a = +44.0;
 a = +44.9;
 a = +12.78e5;
 a = +12e10;
 a = -x;
 a = -17;
 a = -44;
 a = -44.0;
 a = -44.9;
 a = -12.78e5;
 a = -12e10;
 a = +-x;
 a = +-17;
 a = +-44;
 a = +-44.0;
 a = +-44.9;
 a = +-12.78e5;
 a = +-12e10;
 a = +0x8000000000000000;
 a = +-0x8000000000000000;
 a = -+0x8000000000000000;
 a = -0x8000000000000000;
 a = +0xde0b6b000000000;
 a = +-0xde0b6b000000000;
 a = -+0xde0b6b000000000;
 a = -0xde0b6b000000000;
 a = +0x3ce7184d470dd60000;
 f(g() & -1);
 x = 1.7976931348623157e+308;
 a = 9007199254740992; // 2^53, the largest integer that can be represented in a double such that all smaller integers are also representable.
 a = 9007199254740993; // 2^53 + 1 cannot be represented as double.
 a = 9007199254740994; // This is again representable as double.
 return +12e10;
}
function looop() {
 while (1) {
  do_it();
  if (condition()) {
   break;
  }
 }
 while (1) {
  do_it();
  if (a > b) {
   break;
  }
 }
 while (1) {
  do_it();
  if (!x()) {
   break;
  }
 }
 while (1) {
  do_it();
  if (a()) continue; // we cannot move to do-while, continue will hit the while check
  if (!x()) {
   break;
  }
 }
 while (1) {
  do_it();
  do {
    if (a()) continue; // ok to optimize, continue is not for us
  } while (b());
  if (!x()) {
   break;
  }
 }
 while (1) {
  do_it();
  while (b()) {
    if (a()) continue; // also ok to optimize, continue is not for us
  }
  if (!x()) {
   break;
  }
 }
 X: while (1) {
  do_it();
  while (b()) {
    if (a()) continue X; // not ok to optimize
  }
  if (!x()) {
   break;
  }
 }
 while (1) {
  blah();
  if (shah()) {
   a = b;
   break;
  }
 }
 LABELED: while (1) {
  blah();
  if (shah()) {
   c = d;
   break;
  }
 }
 while (1) {
  blah();
  if (check) break; // prevents optimization
  if (shah()) {
   e = f;
   break;
  }
 }
 while (1) {
  blah();
  while (1) {
   if (check) break; // safe to optimize
  }
  if (shah()) {
   g = h;
   break;
  }
 }
 if (a) { waka(); }
 if (a) { waka(); } else { wala(); }
 if (a) { if (a) { waka(); } else { wala(); } }
 if (a) { if (a) { waka(); } } else { other(); }
 if (a) { if (a) { waka(); } else { wala(); } } else { other(); }
}
function conditions() {
 if (!((HEAP32[$incdec_ptr71_i + 8 >> 2] | 0) == 0)) {
  shoo();
 }
 if (x == 0) y();
 if ((x | 0) == 0) y();
 if (0 == (y | 0)) z();
 if (x != 0) y();
 if ((x | 0) != 0) y();
 if (0 != (y | 0)) z();
 if (!((x | 0) == 0)) y();
 if (!(0 != (x | 0))) y();
 if (!(s() | 0)) z();
 if ((x + 4 | 0) != 0) y();
 if ((x & 4 | 0) != 0) y();
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["finall", "looop", "conditions"]

