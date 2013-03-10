function asm(d1, i2) {
  d1 = +d1;
  i2 = i2 | 0;
  var i3 = 0, d4 = +0;
  i2 = d1 + d1 | 0;
  d1 = d(Math_max(10, Math_min(5, f())));
  i3 = i2 + 2 | 0;
  print(i3);
  d4 = d1 * 5;
  return d4;
}
function _doit(i1, i2, i3) {
  i1 = i1 | 0;
  i2 = i2 | 0;
  i3 = i3 | 0;
  i1 = STACKTOP;
  _printf(__str | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[(tempInt & 16777215) >> 2] = i2, HEAP32[(tempInt + 4 & 16777215) >> 2] = i3, tempInt));
  STACKTOP = i1;
  return 0 | 0;
}
function stackRestore(i1) {
  i1 = i1 | 0;
  STACKTOP = i1;
}

