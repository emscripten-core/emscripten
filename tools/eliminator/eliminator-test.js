function a() {
  var $210 = HEAP32[100]; // heaps alias each other! so this cannot be eliminated
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $210;
  chak();
  var $210a = HEAP32[100]; // function calls can also modify memory
  something();
  HEAP32[90] = $210a;
  chak();
  var $a = $hack; // no mem use (just a global), so ok to eliminate
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $a;
  chak();
  var $bb = HEAP32[11]; // ok to eliminate
  var $b = ($bb+7)|0; // ok to eliminate by itself, but not with inlined $bb which is mem-using!
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $b;
  chak();
  var $bb2 = HEAP32[11];
  HEAP32[111] = 321;
  var $b2 = ($bb2+7)|0;
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $b2;
  chak();
  var $d = HEAP32[100]; // alias on next line, but that is where we are consumed - so ok.
  HEAP32[1e3] = $d;
  chak();
  var $e = func();
  HEAP32[1e3] = $e;
  chak();
  var $e2 = func();
  tor($e2);
  chak();
  var $e3 = HEAP[9];
  tor($e3);
  barrier(); // same stuff, but with a var on top and assigns as the first and only def
  var $$210, $$210a, $$a, $$bb, $$b, $$bb2, $$b2, $$d, $$e, $$e2, $$e3;
  $$210 = HEAP32[100]; // heaps alias each other! so this cannot be eliminated
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $$210;
  chak();
  $$210a = HEAP32[100]; // function calls can also modify memory
  something();
  HEAP32[90] = $$210a;
  chak();
  $$a = $$hack; // no mem use, so ok to eliminate
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $$a;
  chak();
  $$bb = HEAP32[11]; // ok to eliminate
  $$b = ($$bb+7)|0; // ok to eliminate by itself, but not with inlined $$bb which is mem-using!
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $$b;
  chak();
  $$bb2 = HEAP32[11];
  HEAP32[111] = 321;
  $$b2 = ($$bb2+7)|0;
  HEAP32[1e3] = HEAP32[5];
  HEAP32[90] = $$b2;
  chak();
  $$d = HEAP32[100]; // alias on next line, but that is where we are consumed - so ok.
  HEAP32[1e3] = $$d;
  chak();
  $$e = func();
  HEAP32[1e3] = $$e;
  chak();
  $$e2 = func();
  tor($$e2);
  chak();
  $$e3 = HEAP[9];
  tor($$e3);
  // TODO: (,a,b) 'seq'
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["f", "g", "h", "py", "r", "t", "f2", "f3", "llvm3_1", "_inflate", "_malloc", "a"]
