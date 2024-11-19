// EMSCRIPTEN_START_ASM
function instantiate(q, r, s) {
  var a = new ArrayBuffer(16);
  var b = new Int32Array(a);
  var c = new Float32Array(a);
  function d(e) {
    return b[e];
  }
  function f(e, g) {
    b[e] = g;
  }
  function h(g) {
    c[2] = g;
  }
  function i() {
    return c[2];
  }
  function m(n, o, p) {
    var j = new n.Int8Array(p);
    var k = o.emscripten_glVertexAttrib4fv;
    var l = 6191184;
    
// EMSCRIPTEN_START_FUNCS
function v() {
  return t(10, 20) + u(30);
}
function t(a, b) {
  return u(a + b);
}

function u(a) {
  return a + 1;
}


// EMSCRIPTEN_END_FUNCS

    return {
      main: v
    };
  }
  return m({
    Int8Array,
    Int16Array,
    Int32Array,
    Uint8Array,
    Uint16Array,
    Uint32Array,
    Float32Array,
    Float64Array,
    NaN,
    Infinity: Infinity,
    Math
  }, q, r.buffer);
}
// EMSCRIPTEN_END_ASM

