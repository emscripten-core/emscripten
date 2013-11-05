function a($directory) {
 var $1 = _strlen($directory);
 var $p_0 = $directory + $1 | 0;
 chak($p_0);
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
 barrier();
 var $65, $image, $51$s2, $71;
 var $66 = HEAP32[$65 >> 2];
 var $71 = $66 - _int_ceildiv(HEAP32[$image >> 2], HEAP32[$51$s2]) | 0;
 HEAP32[$65 >> 2] = _int_ceildivpow2($71, HEAP32[$51$s2 + 10]);
 barr();
 var ONCE = sheep();
 while (ONCE) {
  work();
 }
 var ONCEb = 75;
 while (ONCEb) {
  work();
 }
 var $26 = __ZL3minIiET_S0_S0_12(4096, 4096 - $16 | 0); // cannot eliminate this because the call might modify FUNCTION_TABLE
 var $27 = FUNCTION_TABLE[$22]($18, $this + ($16 + 27) | 0, $26);
 print($27);
 chak();
 var zzz = 10;
 do {
  print(zzz);
 } while (0);
 var zzz1 = 10;
 do {
  print(zzz1);
 } while (1); // cannot eliminate a do-while that is not one-time
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a"]

