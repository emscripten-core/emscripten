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
function _doit($x, $y$0, $y$1) {
  $x = $x | 0;
  $y$0 = $y$0 | 0;
  $y$1 = $y$1 | 0;
  var __stackBase__ = 0;
  __stackBase__ = STACKTOP;
  _printf(__str | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[(tempInt & 16777215) >> 2] = $y$0, HEAP32[(tempInt + 4 & 16777215) >> 2] = $y$1, tempInt));
  STACKTOP = __stackBase__;
  return 0 | 0;
}
function stackRestore(top) {
  top = top|0;
  STACKTOP = top;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "_doit", "stackRestore"]

