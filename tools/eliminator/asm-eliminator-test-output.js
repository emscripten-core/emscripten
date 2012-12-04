function asm(x, y) {
  x = +x;
  y = y | 0;
  var a = 0;
  a = cheez(y + ~~x | 0) | 0;
  fleefl(a * a | 0, a | 0);
}
function __Z11printResultPiS_j($needle, $haystack, $len) {
  $needle = $needle | 0;
  $haystack = $haystack | 0;
  $len = $len | 0;
  var $3 = 0, __stackBase__ = 0;
  __stackBase__ = STACKTOP;
  $3 = _bsearch($needle, $haystack, $len, 4, 2);
  if (($3 | 0) == 0) {
    _puts(_str | 0);
    STACKTOP = __stackBase__;
    return;
  } else {
    _printf(__str1 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[(tempInt & 16777215) >> 2] = HEAP32[($3 & 16777215) >> 2] | 0, tempInt));
    STACKTOP = __stackBase__;
    return;
  }
}

