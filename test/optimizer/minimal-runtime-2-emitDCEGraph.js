var Module = Module;
function ready() {
 run();
}
var UTF8Decoder = new TextDecoder("utf8");
function UTF8ArrayToString(u8Array, idx) {
 var endPtr = idx;
 while (u8Array[endPtr]) ++endPtr;
 return UTF8Decoder.decode(u8Array.subarray ? u8Array.subarray(idx, endPtr) : new Uint8Array(u8Array.slice(idx, endPtr)));
}
function UTF8ToString(ptr) {
 return ptr ? UTF8ArrayToString(HEAPU8, ptr) : "";
}
var TOTAL_MEMORY = 16777216, STATIC_BASE = 1024;
var wasmMaximumMemory = TOTAL_MEMORY;
var wasmMemory = new WebAssembly.Memory({
 "initial": TOTAL_MEMORY >> 16,
 "maximum": wasmMaximumMemory >> 16
});
var buffer = wasmMemory.buffer;
var HEAP8 = new Int8Array(buffer);
var HEAP16 = new Int16Array(buffer);
var HEAP32 = new Int32Array(buffer);
var HEAPU8 = new Uint8Array(buffer);
var HEAPU16 = new Uint16Array(buffer);
var HEAPU32 = new Uint32Array(buffer);
var HEAPF32 = new Float32Array(buffer);
var HEAPF64 = new Float64Array(buffer);
var __ATINIT__ = [];
function _emscripten_console_log(str) {
 console.log(UTF8ToString(str));
}
var wasmImports = {
 "a": _emscripten_console_log
};
function run() {
 var ret = _main();
}
function initRuntime() {
 for (var i in __ATINIT__) __ATINIT__[i].func();
}
var env = wasmImports;
env["memory"] = wasmMemory;
env["table"] = new WebAssembly.Table({
 "initial": 0,
 "maximum": 0,
 "element": "anyfunc"
});
env["__memory_base"] = STATIC_BASE;
env["__table_base"] = 0;
var imports = {
 "env": env,
 "global": {
  "NaN": NaN,
  "Infinity": Infinity
 },
 "global.Math": Math,
 "asm2wasm": {
  "f64-rem": (function(x, y) {
   return x % y;
  }),
  "debugger": (function() {
   debugger;
  })
 }
};
var _main, _unused, wasmExports;
WebAssembly.instantiate(Module["wasm"], imports).then(((output) => {
 wasmExports = output.instance.exports;
 _main = wasmExports["b"];
 _unused = wasmExports["c"];
 initRuntime();
 ready();
}));

// EXTRA_INFO: { "exports": [["_main","_main"]]}
