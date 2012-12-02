function asm(x, y) {
  x = +x;
  y = y|0;
  var a = 0, b = +0, c = 0;
  var label = 0;
  a = cheez((y+~~x)|0)|0;
  b = a*a;
  fleefl(b|0, a|0);
}
function __Z11printResultPiS_j($needle, $haystack, $len) {
  $needle = $needle | 0;
  $haystack = $haystack | 0;
  $len = $len | 0;
  var $1 = 0, $2 = 0, $3 = 0, $4 = 0, $puts = 0, $7 = 0, $8 = 0, $9 = 0;
  var label = 0;
  var __stackBase__ = STACKTOP;
  $1 = $needle;
  $2 = $haystack;
  $3 = _bsearch($1, $2, $len, 4, 2);
  $4 = ($3 | 0) == 0;
  if ($4) {
    $puts = _puts(_str | 0);
    STACKTOP = __stackBase__;
    return;
  } else {
    $7 = $3;
    $8 = HEAP32[($7 & 16777215) >> 2] | 0;
    $9 = _printf(__str1 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[(tempInt & 16777215) >> 2] = $8, tempInt));
    STACKTOP = __stackBase__;
    return;
  }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["asm", "__Z11printResultPiS_j"]

