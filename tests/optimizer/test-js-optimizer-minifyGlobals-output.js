function instantiate(Q) {
 function N(R) {
  R.set = (function(S, T) {
   this[S] = T;
  });
  R.get = (function(S) {
   return this[S];
  });
  return R;
 }
 function O(U) {
  var a = U.a;
  var b = a.buffer;
  var c = new Int8Array(b);
  var d = new Int16Array(b);
  var e = new Int32Array(b);
  var f = new Uint8Array(b);
  var g = new Uint16Array(b);
  var h = new Uint32Array(b);
  var i = new Float32Array(b);
  var j = new Float64Array(b);
  var k = Math.imul;
  var l = Math.fround;
  var m = Math.abs;
  var n = Math.clz32;
  var o = Math.min;
  var p = Math.max;
  var q = Math.floor;
  var r = Math.ceil;
  var s = Math.trunc;
  var t = Math.sqrt;
  var u = U.abort;
  var v = NaN;
  var w = Infinity;
  var x = U.b;
  var y = U.c;
  var z = 5245136;
  EMSCRIPTEN_FUNCS();
  var A = N([ null, H, I, G ]);
  function P() {
   return b.byteLength / 65536 | 0;
  }
  return {
   "d": D,
   "e": M,
   "f": A
  };
 }
 return O(Q);
}


// EXTRA_INFO:{"memory":"a","buffer":"b","HEAP8":"c","HEAP16":"d","HEAP32":"e","HEAPU8":"f","HEAPU16":"g","HEAPU32":"h","HEAPF32":"i","HEAPF64":"j","Math_imul":"k","Math_fround":"l","Math_abs":"m","Math_clz32":"n","Math_min":"o","Math_max":"p","Math_floor":"q","Math_ceil":"r","Math_trunc":"s","Math_sqrt":"t","abort":"u","nan":"v","infinity":"w","fimport$0":"x","fimport$1":"y","global$0":"z","FUNCTION_TABLE":"A","$0":"B","$1":"C","$2":"D","$3":"E","$4":"F","$5":"G","$6":"H","$7":"I","$8":"J","$9":"K","$10":"L","$11":"M","Table":"N","asmFunc":"O","__wasm_memory_size":"P","asmLibraryArg":"Q","ret":"R","i":"S","func":"T","env":"U"}
