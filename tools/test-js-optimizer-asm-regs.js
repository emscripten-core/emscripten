function asm(x, y) {
  x = +x;
  y = y | 0;
  var int1 = 0, int2 = 0; // do not mix the types!
  var double1 = +0, double2 = +0;
  int1 = (x+x)|0;
  double1 = d(Math.max(10, Math_min(5, f())));
  int2 = (int1+2)|0;
  print(int2);
  double2 = double1*5;
  return double2;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "__Z11printResultPiS_j"]

