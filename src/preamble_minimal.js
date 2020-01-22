#if SEPARATE_ASM && ASSERTIONS && WASM == 0 && MODULARIZE
if (!({{{ASM_MODULE_NAME}}})) throw 'Must load asm.js Module in to variable {{{ASM_MODULE_NAME}}} before adding compiled output .js script to the DOM';
#endif

#include "runtime_safe_heap.js"

#if ASSERTIONS
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) throw text;
}
#endif

function abort(what) {
  throw what;
}

var tempRet0 = 0;
var setTempRet0 = function(value) {
  tempRet0 = value;
}
var getTempRet0 = function() {
  return tempRet0;
}

function alignUp(x, multiple) {
  if (x % multiple > 0) {
    x += multiple - (x % multiple);
  }
  return x;
}

#include "runtime_strings.js"
#include "runtime_sab_polyfill.js"

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif

var GLOBAL_BASE = {{{ GLOBAL_BASE }}},
    TOTAL_STACK = {{{ TOTAL_STACK }}},
    STATIC_BASE = {{{ GLOBAL_BASE }}},
    STACK_BASE = {{{ getQuoted('STACK_BASE') }}},
    STACKTOP = STACK_BASE,
    STACK_MAX = {{{ getQuoted('STACK_MAX') }}}
#if USES_DYNAMIC_ALLOC
    , DYNAMICTOP_PTR = {{{ DYNAMICTOP_PTR }}};
#endif
    ;

#if WASM

#if ALLOW_MEMORY_GROWTH && WASM_MEM_MAX != -1
var wasmMaximumMemory = {{{ WASM_MEM_MAX }}};
#else
var wasmMaximumMemory = {{{ TOTAL_MEMORY }}};
#endif

var wasmMemory = new WebAssembly.Memory({
  'initial': {{{ TOTAL_MEMORY }}} >> 16
#if USE_PTHREADS || !ALLOW_MEMORY_GROWTH || WASM_MEM_MAX != -1
  , 'maximum': wasmMaximumMemory >> 16
#endif
#if USE_PTHREADS
  , 'shared': true
#endif
  });

var wasmTable = new WebAssembly.Table({
  'initial': {{{ getQuoted('WASM_TABLE_SIZE') }}},
#if !ALLOW_TABLE_GROWTH
#if WASM_BACKEND
  'maximum': {{{ getQuoted('WASM_TABLE_SIZE') }}} + {{{ RESERVED_FUNCTION_POINTERS }}},
#else
  'maximum': {{{ getQuoted('WASM_TABLE_SIZE') }}},
#endif
#endif // WASM_BACKEND
  'element': 'anyfunc'
});

var buffer = wasmMemory.buffer;

#if USE_PTHREADS && ASSERTIONS
assert(buffer instanceof SharedArrayBuffer, 'requested a shared WebAssembly.Memory but the returned buffer is not a SharedArrayBuffer, indicating that while the browser has SharedArrayBuffer it does not have WebAssembly threads support - you may need to set a flag');
#endif

#else

#if USE_PTHREADS
var buffer = new SharedArrayBuffer({{{ TOTAL_MEMORY }}});
#else
var buffer = new ArrayBuffer({{{ TOTAL_MEMORY }}});
#endif

#if USE_PTHREADS
}
#endif

#endif

#if ASSERTIONS
var WASM_PAGE_SIZE = 65536;
assert(STACK_BASE % 16 === 0, 'stack must start aligned');
assert(({{{ getQuoted('DYNAMIC_BASE') }}}) % 16 === 0, 'heap must start aligned');
assert({{{ TOTAL_MEMORY }}} >= TOTAL_STACK, 'TOTAL_MEMORY should be larger than TOTAL_STACK, was ' + {{{ TOTAL_MEMORY }}} + '! (TOTAL_STACK=' + TOTAL_STACK + ')');
assert({{{ TOTAL_MEMORY }}} % WASM_PAGE_SIZE === 0);
#if WASM_MEM_MAX != -1
assert({{{ WASM_MEM_MAX }}} % WASM_PAGE_SIZE == 0);
#endif
assert(buffer.byteLength === {{{ TOTAL_MEMORY }}});
#endif // ASSERTIONS

#if ALLOW_MEMORY_GROWTH
// In ALLOW_MEMORY_GROWTH, we need to be able to re-initialize the
// typed array buffer and heap views to the buffer whenever the heap
// is resized.
var HEAP8, HEAP16, HEAP32, HEAPU8, HEAPU16, HEAPU32, HEAPF32, HEAPF64;
function updateGlobalBufferAndViews(b) {
  buffer = b;
  HEAP8 = new Int8Array(b);
  HEAP16 = new Int16Array(b);
  HEAP32 = new Int32Array(b);
  HEAPU8 = new Uint8Array(b);
  HEAPU16 = new Uint16Array(b);
  HEAPU32 = new Uint32Array(b);
  HEAPF32 = new Float32Array(b);
  HEAPF64 = new Float64Array(b);
}
updateGlobalBufferAndViews(buffer);
#else
// In non-ALLOW_MEMORY_GROWTH scenario, we only need to initialize
// the heap once, so optimize code size to do it statically here.
var HEAP8 = new Int8Array(buffer);
var HEAP16 = new Int16Array(buffer);
var HEAP32 = new Int32Array(buffer);
var HEAPU8 = new Uint8Array(buffer);
var HEAPU16 = new Uint16Array(buffer);
var HEAPU32 = new Uint32Array(buffer);
var HEAPF32 = new Float32Array(buffer);
var HEAPF64 = new Float64Array(buffer);
#endif

#if !WASM
HEAPU8.set(new Uint8Array(Module['mem']), GLOBAL_BASE);
#endif

#if USES_DYNAMIC_ALLOC
HEAP32[DYNAMICTOP_PTR>>2] = {{{ getQuoted('DYNAMIC_BASE') }}};
#endif

#include "runtime_stack_check.js"
#include "runtime_assertions.js"

#if ASSERTIONS
var runtimeInitialized = false;

// This is always false in minimal_runtime - the runtime does not have a concept of exiting (keeping this variable here for now since it is referenced from generated code)
var runtimeExited = false;
#endif

{{{ unSign }}}
{{{ reSign }}}

#include "runtime_math.js"

var memoryInitializer = null;

#if MEMORYPROFILER
#include "memoryprofiler.js"
#endif

// === Body ===
