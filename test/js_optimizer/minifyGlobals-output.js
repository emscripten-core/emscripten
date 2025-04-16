function instantiate(I) {
  function c(d) {
    d.set = function(a, b) {
      this[a] = b;
    };
    d.get = function(a) {
      return this[a];
    };
    return d;
  }
  function G(H) {
    var e = H.a;
    var f = e.buffer;
    var g = new Int8Array(f);
    var h = new Int16Array(f);
    var i = new Int32Array(f);
    var j = new Uint8Array(f);
    var k = new Uint16Array(f);
    var l = new Uint32Array(f);
    var m = new Float32Array(f);
    var n = new Float64Array(f);
    var o = Math.imul;
    var p = Math.fround;
    var q = Math.abs;
    var r = Math.clz32;
    var s = Math.min;
    var t = Math.max;
    var u = Math.floor;
    var v = Math.ceil;
    var w = Math.trunc;
    var x = Math.sqrt;
    var y = H.abort;
    var z = NaN;
    var A = Infinity;
    var B = H.b;
    var C = H.c;
    var D = 5245136;
    EMSCRIPTEN_FUNCS();
    var E = c([ null, P, Q, O ]);
    function F() {
      return f.byteLength / 65536 | 0;
    }
    return {
      d: L,
      e: U,
      f: E
    };
  }
  return G(I);
}
// EXTRA_INFO:{"i":"a","func":"b","Table":"c","ret":"d","memory":"e","buffer":"f","HEAP8":"g","HEAP16":"h","HEAP32":"i","HEAPU8":"j","HEAPU16":"k","HEAPU32":"l","HEAPF32":"m","HEAPF64":"n","Math_imul":"o","Math_fround":"p","Math_abs":"q","Math_clz32":"r","Math_min":"s","Math_max":"t","Math_floor":"u","Math_ceil":"v","Math_trunc":"w","Math_sqrt":"x","abort":"y","nan":"z","infinity":"A","fimport$0":"B","fimport$1":"C","global$0":"D","FUNCTION_TABLE":"E","__wasm_memory_size":"F","asmFunc":"G","env":"H","wasmImports":"I","$0":"J","$1":"K","$2":"L","$3":"M","$4":"N","$5":"O","$6":"P","$7":"Q","$8":"R","$9":"S","$10":"T","$11":"U"}
