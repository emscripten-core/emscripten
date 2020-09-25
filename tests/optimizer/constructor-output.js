// EMSCRIPTEN_START_ASM
function instantiate(o, p, q) {
 var a = new ArrayBuffer(16);
 var b = new Int32Array(a);
 var c = new Float32Array(a);
 function j(r) {
  return b[r];
 }
 function k(r, s) {
  b[r] = s;
 }
 function l(s) {
  c[2] = s;
 }
 function m() {
  return c[2];
 }
 function n(t, u, v) {
  var d = new t.Int8Array(v);
  var e = u.emscripten_glVertexAttrib4fv;
  var f = 6191184;
  
// EMSCRIPTEN_START_FUNCS
function i() {
 return g(10, 20) + h(30);
}



function g(a, b) {
 return h(a + b);
}
function h(a) {
 return a + 1;
}

// EMSCRIPTEN_END_FUNCS

  return {
   "main": i
  };
 }
 return n({
  "Int8Array": Int8Array,
  "Int16Array": Int16Array,
  "Int32Array": Int32Array,
  "Uint8Array": Uint8Array,
  "Uint16Array": Uint16Array,
  "Uint32Array": Uint32Array,
  "Float32Array": Float32Array,
  "Float64Array": Float64Array,
  "NaN": NaN,
  "Infinity": Infinity,
  "Math": Math
 }, o, p.buffer);
}


// EMSCRIPTEN_END_ASM

// EMSCRIPTEN_GENERATED_FUNCTIONS


