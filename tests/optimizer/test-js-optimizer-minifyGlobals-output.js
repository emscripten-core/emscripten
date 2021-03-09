function instantiate(a) {
 function b(c) {
  c.set = function(d, e) {
   this[d] = e;
  };
  c.get = function(d) {
   return this[d];
  };
  return c;
 }
 function f(g) {
  var h = g.a;
  var i = h.i;
  var j = new Int8Array(i);
  var k = new Int16Array(i);
  var l = new Int32Array(i);
  var m = new Uint8Array(i);
  var n = new Uint16Array(i);
  var o = new Uint32Array(i);
  var p = new Float32Array(i);
  var q = new Float64Array(i);
  var r = Math.imul;
  var s = Math.fround;
  var t = Math.abs;
  var u = Math.clz32;
  var v = Math.min;
  var w = Math.max;
  var x = Math.floor;
  var y = Math.ceil;
  var z = Math.trunc;
  var A = Math.sqrt;
  var B = g.B;
  var C = NaN;
  var D = Infinity;
  var E = g.b;
  var F = g.c;
  var G = 5245136;
  EMSCRIPTEN_FUNCS();
  var H = b([ null, $6, $7, $5 ]);
  function I() {
   return i.byteLength / 65536 | 0;
  }
  return {
   "d": $2,
   "e": $11,
   "f": H
  };
 }
 return f(a);
}
// EXTRA_INFO:{"asmLibraryArg":"a","Table":"b","ret":"c","i":"d","func":"e","asmFunc":"f","env":"g","memory":"h","buffer":"i","HEAP8":"j","HEAP16":"k","HEAP32":"l","HEAPU8":"m","HEAPU16":"n","HEAPU32":"o","HEAPF32":"p","HEAPF64":"q","Math_imul":"r","Math_fround":"s","Math_abs":"t","Math_clz32":"u","Math_min":"v","Math_max":"w","Math_floor":"x","Math_ceil":"y","Math_trunc":"z","Math_sqrt":"A","abort":"B","nan":"C","infinity":"D","fimport$0":"E","fimport$1":"F","global$0":"G","FUNCTION_TABLE":"H","__wasm_memory_size":"I"}
