// EMSCRIPTEN_START_ASM
function instantiate(asmLibraryArg, wasmMemory, wasmTable) {
 var scratchBuffer = new ArrayBuffer(16);
 var i32ScratchView = new Int32Array(scratchBuffer);
 var f32ScratchView = new Float32Array(scratchBuffer);
 function wasm2js_scratch_load_i32(index) {
  return i32ScratchView[index];
 }
 function wasm2js_scratch_store_i32(index, value) {
  i32ScratchView[index] = value;
 }
 function wasm2js_scratch_store_f32(value) {
  f32ScratchView[2] = value;
 }
 function wasm2js_scratch_load_f32() {
  return f32ScratchView[2];
 }
 function asmFunc(global, env, buffer) {
  var a = new global.Int8Array(buffer);
  var b = env.emscripten_glVertexAttrib4fv;
  var c = 6191184;
  
// EMSCRIPTEN_START_FUNCS
function f() {
 return d(10, 20) + e(30);
}



function d(a, b) {
 return e(a + b);
}
function e(a) {
 return a + 1;
}

// EMSCRIPTEN_END_FUNCS

  return {
   "main": f
  };
 }
 return asmFunc({
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


