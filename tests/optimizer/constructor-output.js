// EMSCRIPTEN_START_ASM
function a(asmLibraryArg, wasmMemory, wasmTable) {
 var scratchBuffer = new ArrayBuffer(16);
 var b = new Int32Array(scratchBuffer);
 var c = new Float32Array(scratchBuffer);
 function d(index) {
  return b[index];
 }
 function e(index, value) {
  b[index] = value;
 }
 function f(value) {
  c[2] = value;
 }
 function g() {
  return c[2];
 }
 function h(global, env, buffer) {
  var i = new global.Int8Array(buffer);
  var j = env.emscripten_glVertexAttrib4fv;
  var k = 6191184;
  
// EMSCRIPTEN_START_FUNCS
function n() {
 return l(10, 20) + m(30);
}



function l(a, b) {
 return m(a + b);
}
function m(a) {
 return a + 1;
}

// EMSCRIPTEN_END_FUNCS

  return {
   "main": n
  };
 }
 return h({
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
 }, asmLibraryArg, wasmMemory.buffer);
}


// EMSCRIPTEN_END_ASM

// EMSCRIPTEN_GENERATED_FUNCTIONS


