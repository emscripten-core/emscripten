// === Preamble/preparation code ===
//
// The code in this file sets up memory and other preparations for running
// the compiled code. The code here is mostly self-contained, with little
// other code depending on it - it is expected that in time this will be
// customizable, specifically, replaceable with custom user code (that may
// be much more specialized and minimal).

// The address globals begin at. Very low in memory, for code size and optimization opportunities.
// Above 0 is static memory, starting with globals.
// Then the stack.
// Then 'dynamic' memory for sbrk.
var GLOBAL_BASE = {{{ GLOBAL_BASE }}};

#if RELOCATABLE
GLOBAL_BASE = alignMemory(GLOBAL_BASE, {{{ MAX_GLOBAL_ALIGN || 1 }}});
#endif

#if BENCHMARK
Module.realPrint = out;
out = err = function(){};
#endif

// Essentials

var ABORT = 0; // whether we are quitting the application. no code should run after this. set in exit() and abort()
var EXITSTATUS = 0;

// Memory management

var PAGE_SIZE = 16384;
var WASM_PAGE_SIZE = 65536;
var ASMJS_PAGE_SIZE = 16777216;
var MIN_TOTAL_MEMORY = 16777216;

function updateGlobalBuffer(buf) {
  Module['buffer'] = buffer = buf;
}

function updateGlobalBufferViews() {
  Module['HEAP8'] = HEAP8 = new Int8Array(buffer);
  Module['HEAP16'] = HEAP16 = new Int16Array(buffer);
  Module['HEAP32'] = HEAP32 = new Int32Array(buffer);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buffer);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buffer);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buffer);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buffer);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buffer);
}

var STATIC_BASE, STATICTOP, staticSealed; // static area
var STACK_BASE, STACKTOP, STACK_MAX; // stack area
var DYNAMIC_BASE, DYNAMICTOP_PTR; // dynamic area handled by sbrk

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) { // Pthreads have already initialized these variables in src/pthread-main.js, where they were passed to the thread worker at startup time
#endif
  STATIC_BASE = STATICTOP = STACK_BASE = STACKTOP = STACK_MAX = DYNAMIC_BASE = DYNAMICTOP_PTR = 0;
  staticSealed = false;
#if USE_PTHREADS
}
#endif

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) {
  staticSealed = true; // The static memory area has been initialized already in the main thread, pthreads skip this.
#if SEPARATE_ASM != 0
  importScripts('{{{ SEPARATE_ASM }}}'); // load the separated-out asm.js
#endif
}
#endif

#if ABORTING_MALLOC
function abortOnCannotGrowMemory() {
#if WASM
  abort('Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value ' + TOTAL_MEMORY + ', (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ');
#else
  abort('Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value ' + TOTAL_MEMORY + ', (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime but prevents some optimizations, (3) set Module.TOTAL_MEMORY to a higher value before the program runs, or (4) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ');
#endif
}
#endif

#if ALLOW_MEMORY_GROWTH
if (!Module['reallocBuffer']) Module['reallocBuffer'] = function(size) {
  var ret;
  try {
    if (ArrayBuffer.transfer) {
      ret = ArrayBuffer.transfer(buffer, size);
    } else {
      var oldHEAP8 = HEAP8;
      ret = new ArrayBuffer(size);
      var temp = new Int8Array(ret);
      temp.set(oldHEAP8);
    }
  } catch(e) {
    return false;
  }
  var success = _emscripten_replace_memory(ret);
  if (!success) return false;
  return ret;
};
#endif

function enlargeMemory() {
#if USE_PTHREADS
  abort('Cannot enlarge memory arrays, since compiling with pthreads support enabled (-s USE_PTHREADS=1).');
#else
#if ALLOW_MEMORY_GROWTH == 0
#if ABORTING_MALLOC
  abortOnCannotGrowMemory();
#else
  return false; // malloc will report failure
#endif
#else
  // TOTAL_MEMORY is the current size of the actual array, and DYNAMICTOP is the new top.
#if ASSERTIONS
  assert(HEAP32[DYNAMICTOP_PTR>>2] > TOTAL_MEMORY); // This function should only ever be called after the ceiling of the dynamic heap has already been bumped to exceed the current total size of the asm.js heap.
#endif

#if EMSCRIPTEN_TRACING
  // Report old layout one last time
  _emscripten_trace_report_memory_layout();
#endif

  var PAGE_MULTIPLE = Module["usingWasm"] ? WASM_PAGE_SIZE : ASMJS_PAGE_SIZE; // In wasm, heap size must be a multiple of 64KB. In asm.js, they need to be multiples of 16MB.
  var LIMIT = 2147483648 - PAGE_MULTIPLE; // We can do one page short of 2GB as theoretical maximum.

  if (HEAP32[DYNAMICTOP_PTR>>2] > LIMIT) {
#if ASSERTIONS
    err('Cannot enlarge memory, asked to go up to ' + HEAP32[DYNAMICTOP_PTR>>2] + ' bytes, but the limit is ' + LIMIT + ' bytes!');
#endif
    return false;
  }

  var OLD_TOTAL_MEMORY = TOTAL_MEMORY;
  TOTAL_MEMORY = Math.max(TOTAL_MEMORY, MIN_TOTAL_MEMORY); // So the loop below will not be infinite, and minimum asm.js memory size is 16MB.

  while (TOTAL_MEMORY < HEAP32[DYNAMICTOP_PTR>>2]) { // Keep incrementing the heap size as long as it's less than what is requested.
    if (TOTAL_MEMORY <= 536870912) {
      TOTAL_MEMORY = alignUp(2 * TOTAL_MEMORY, PAGE_MULTIPLE); // Simple heuristic: double until 1GB...
    } else {
      // ..., but after that, add smaller increments towards 2GB, which we cannot reach
      TOTAL_MEMORY = Math.min(alignUp((3 * TOTAL_MEMORY + 2147483648) / 4, PAGE_MULTIPLE), LIMIT);
#if ASSERTIONS
      if (TOTAL_MEMORY === OLD_TOTAL_MEMORY) {
        warnOnce('Cannot ask for more memory since we reached the practical limit in browsers (which is just below 2GB), so the request would have failed. Requesting only ' + TOTAL_MEMORY);
      }
#endif
    }
  }

#if ASSERTIONS
  var start = Date.now();
#endif

  var replacement = Module['reallocBuffer'](TOTAL_MEMORY);
  if (!replacement || replacement.byteLength != TOTAL_MEMORY) {
#if ASSERTIONS
    err('Failed to grow the heap from ' + OLD_TOTAL_MEMORY + ' bytes to ' + TOTAL_MEMORY + ' bytes, not enough memory!');
    if (replacement) {
      err('Expected to get back a buffer of size ' + TOTAL_MEMORY + ' bytes, but instead got back a buffer of size ' + replacement.byteLength);
    }
#endif
    // restore the state to before this call, we failed
    TOTAL_MEMORY = OLD_TOTAL_MEMORY;
    return false;
  }

  // everything worked

  updateGlobalBuffer(replacement);
  updateGlobalBufferViews();

#if ASSERTIONS
  if (!Module["usingWasm"]) {
    err('Warning: Enlarging memory arrays, this is not fast! ' + [OLD_TOTAL_MEMORY, TOTAL_MEMORY]);
  }
#endif

#if EMSCRIPTEN_TRACING
  _emscripten_trace_js_log_message("Emscripten", "Enlarging memory arrays from " + OLD_TOTAL_MEMORY + " to " + TOTAL_MEMORY);
  // And now report the new layout
  _emscripten_trace_report_memory_layout();
#endif

  return true;
#endif // ALLOW_MEMORY_GROWTH
#endif // USE_PTHREADS
}

#if ALLOW_MEMORY_GROWTH
var byteLength;
try {
  byteLength = Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
  byteLength(new ArrayBuffer(4)); // can fail on older ie
} catch(e) { // can fail on older node/v8
  byteLength = function(buffer) { return buffer.byteLength; };
}
#endif

var TOTAL_STACK = Module['TOTAL_STACK'] || {{{ TOTAL_STACK }}};
var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || {{{ TOTAL_MEMORY }}};
if (TOTAL_MEMORY < TOTAL_STACK) err('TOTAL_MEMORY should be larger than TOTAL_STACK, was ' + TOTAL_MEMORY + '! (TOTAL_STACK=' + TOTAL_STACK + ')');

// Initialize the runtime's memory
#if ASSERTIONS
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array !== 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray !== undefined && Int32Array.prototype.set !== undefined,
       'JS engine does not provide full typed array support');
#endif

#if IN_TEST_HARNESS

// Test runs in browsers should always be free from uncaught exceptions. If an uncaught exception is thrown, we fail browser test execution in the REPORT_RESULT() macro to output an error value.
if (ENVIRONMENT_IS_WEB) {
  window.addEventListener('error', function(e) {
    if (e.message.indexOf('SimulateInfiniteLoop') != -1) return;
    console.error('Page threw an exception ' + e);
    Module['pageThrewException'] = true;
  });
}

#if USE_PTHREADS == 1
if (typeof SharedArrayBuffer === 'undefined' || typeof Atomics === 'undefined') {
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:8888/report_result?skipped:%20SharedArrayBuffer%20is%20not%20supported!');
  xhr.send();
  setTimeout(function() { window.close() }, 2000);
}
#endif
#endif

#if USE_PTHREADS
#if !WASM
if (typeof SharedArrayBuffer !== 'undefined') {
  if (!ENVIRONMENT_IS_PTHREAD) buffer = new SharedArrayBuffer(TOTAL_MEMORY);
  // Currently SharedArrayBuffer does not have a slice() operation, so polyfill it in.
  // Adapted from https://github.com/ttaubert/node-arraybuffer-slice, (c) 2014 Tim Taubert <tim@timtaubert.de>
  // arraybuffer-slice may be freely distributed under the MIT license.
  (function (undefined) {
    "use strict";
    function clamp(val, length) {
      val = (val|0) || 0;
      if (val < 0) return Math.max(val + length, 0);
      return Math.min(val, length);
    }
    if (typeof SharedArrayBuffer !== 'undefined' && !SharedArrayBuffer.prototype.slice) {
      SharedArrayBuffer.prototype.slice = function (from, to) {
        var length = this.byteLength;
        var begin = clamp(from, length);
        var end = length;
        if (to !== undefined) end = clamp(to, length);
        if (begin > end) return new ArrayBuffer(0);
        var num = end - begin;
        var target = new ArrayBuffer(num);
        var targetArray = new Uint8Array(target);
        var sourceArray = new Uint8Array(this, begin, num);
        targetArray.set(sourceArray);
        return target;
      };
    }
  })();
} else {
  if (!ENVIRONMENT_IS_PTHREAD) buffer = new ArrayBuffer(TOTAL_MEMORY);
}
updateGlobalBufferViews();

if (typeof Atomics === 'undefined') {
  // Polyfill singlethreaded atomics ops from http://lars-t-hansen.github.io/ecmascript_sharedmem/shmem.html#Atomics.add
  // No thread-safety needed since we don't have multithreading support.
  Atomics = {};
  Atomics['add'] = function(t, i, v) { var w = t[i]; t[i] += v; return w; }
  Atomics['and'] = function(t, i, v) { var w = t[i]; t[i] &= v; return w; }
  Atomics['compareExchange'] = function(t, i, e, r) { var w = t[i]; if (w == e) t[i] = r; return w; }
  Atomics['exchange'] = function(t, i, v) { var w = t[i]; t[i] = v; return w; }
  Atomics['wait'] = function(t, i, v, o) { if (t[i] != v) return 'not-equal'; else return 'timed-out'; }
  Atomics['wake'] = function(t, i, c) { return 0; }
  Atomics['wakeOrRequeue'] = function(t, i1, c, i2, v) { return 0; }
  Atomics['isLockFree'] = function(s) { return true; }
  Atomics['load'] = function(t, i) { return t[i]; }
  Atomics['or'] = function(t, i, v) { var w = t[i]; t[i] |= v; return w; }
  Atomics['store'] = function(t, i, v) { t[i] = v; return v; }
  Atomics['sub'] = function(t, i, v) { var w = t[i]; t[i] -= v; return w; }
  Atomics['xor'] = function(t, i, v) { var w = t[i]; t[i] ^= v; return w; }
}

#else
if (!ENVIRONMENT_IS_PTHREAD) {
#if ALLOW_MEMORY_GROWTH
  Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE , 'maximum': {{{ WASM_MEM_MAX }}} / WASM_PAGE_SIZE, 'shared': true });
#else
  Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE , 'maximum': TOTAL_MEMORY / WASM_PAGE_SIZE, 'shared': true });
#endif
  buffer = Module['wasmMemory'].buffer;
}

updateGlobalBufferViews();
#endif // !WASM
#else // USE_PTHREADS

#if SPLIT_MEMORY == 0
// Use a provided buffer, if there is one, or else allocate a new one
if (Module['buffer']) {
  buffer = Module['buffer'];
#if ASSERTIONS
  assert(buffer.byteLength === TOTAL_MEMORY, 'provided buffer should be ' + TOTAL_MEMORY + ' bytes, but it is ' + buffer.byteLength);
#endif
} else {
  // Use a WebAssembly memory where available
#if WASM
  if (typeof WebAssembly === 'object' && typeof WebAssembly.Memory === 'function') {
#if ASSERTIONS
    assert(TOTAL_MEMORY % WASM_PAGE_SIZE === 0);
#endif // ASSERTIONS
#if ALLOW_MEMORY_GROWTH
#if WASM_MEM_MAX != -1
#if ASSERTIONS
    assert({{{ WASM_MEM_MAX }}} % WASM_PAGE_SIZE == 0);
#endif
    Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE, 'maximum': {{{ WASM_MEM_MAX }}} / WASM_PAGE_SIZE });
#else
    Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE });
#endif // BINARYEN_MEM_MAX
#else
    Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE, 'maximum': TOTAL_MEMORY / WASM_PAGE_SIZE });
#endif // ALLOW_MEMORY_GROWTH
    buffer = Module['wasmMemory'].buffer;
  } else
#endif // WASM
  {
    buffer = new ArrayBuffer(TOTAL_MEMORY);
  }
#if ASSERTIONS
  assert(buffer.byteLength === TOTAL_MEMORY);
#endif // ASSERTIONS
  Module['buffer'] = buffer;
}
updateGlobalBufferViews();
#else // SPLIT_MEMORY
// make sure total memory is a multiple of the split memory size
var SPLIT_MEMORY = {{{ SPLIT_MEMORY }}};
var SPLIT_MEMORY_MASK = SPLIT_MEMORY - 1;
var SPLIT_MEMORY_BITS = -1;
var ALLOW_MEMORY_GROWTH = {{{ ALLOW_MEMORY_GROWTH }}};
var ABORTING_MALLOC = {{{ ABORTING_MALLOC }}};

Module['SPLIT_MEMORY'] = SPLIT_MEMORY;

totalMemory = TOTAL_MEMORY;
if (totalMemory % SPLIT_MEMORY) {
  totalMemory += SPLIT_MEMORY - (totalMemory % SPLIT_MEMORY);
}
if (totalMemory === SPLIT_MEMORY) totalMemory *= 2;
if (totalMemory !== TOTAL_MEMORY) {
  TOTAL_MEMORY = totalMemory;
#if ASSERTIONS == 2
  err('increasing TOTAL_MEMORY to ' + TOTAL_MEMORY + ' to be a multiple>1 of the split memory size ' + SPLIT_MEMORY + ')');
#endif
}

var buffers = [], HEAP8s = [], HEAP16s = [], HEAP32s = [], HEAPU8s = [], HEAPU16s = [], HEAPU32s = [], HEAPF32s = [], HEAPF64s = [];

// Allocates a split chunk, a range of memory of size SPLIT_MEMORY. Generally data is not provided, and a new
// buffer is allocated, this is what happens when malloc works. However, you can provide your own buffer,
// which then lets you access it at address [ i*SPLIT_MEMORY, (i+1)*SPLIT_MEMORY ).
// The function returns true if it succeeds. It can also throw an exception if no data is provided and
// the browser fails to allocate the buffer.
function allocateSplitChunk(i, data) {
  if (buffers[i]) return false; // already taken
  // any of these allocations might fail; do them all before writing anything to global state
  var currBuffer = data ? data : new ArrayBuffer(SPLIT_MEMORY);
#if ASSERTIONS
  assert(currBuffer instanceof ArrayBuffer);
#endif
  var currHEAP8s = new Int8Array(currBuffer);
  var currHEAP16s = new Int16Array(currBuffer);
  var currHEAP32s = new Int32Array(currBuffer);
  var currHEAPU8s = new Uint8Array(currBuffer);
  var currHEAPU16s = new Uint16Array(currBuffer);
  var currHEAPU32s = new Uint32Array(currBuffer);
  var currHEAPF32s = new Float32Array(currBuffer);
  var currHEAPF64s = new Float64Array(currBuffer);
  buffers[i] = currBuffer;
  HEAP8s[i] = currHEAP8s;
  HEAP16s[i] = currHEAP16s;
  HEAP32s[i] = currHEAP32s;
  HEAPU8s[i] = currHEAPU8s;
  HEAPU16s[i] = currHEAPU16s;
  HEAPU32s[i] = currHEAPU32s;
  HEAPF32s[i] = currHEAPF32s;
  HEAPF64s[i] = currHEAPF64s;
  return true;
}
function freeSplitChunk(i) {
#if ASSERTIONS
  assert(buffers[i] && HEAP8s[i]);
  assert(i > 0); // cannot free the first chunk
#endif
  buffers[i] = HEAP8s[i] = HEAP16s[i] = HEAP32s[i] = HEAPU8s[i] = HEAPU16s[i] = HEAPU32s[i] = HEAPF32s[i] = HEAPF64s[i] = null;
}

(function() {
  for (var i = 0; i < TOTAL_MEMORY / SPLIT_MEMORY; i++) {
    buffers[i] = HEAP8s[i] = HEAP16s[i] = HEAP32s[i] = HEAPU8s[i] = HEAPU16s[i] = HEAPU32s[i] = HEAPF32s[i] = HEAPF64s[i] = null;
  }

  var temp = SPLIT_MEMORY;
  while (temp) {
    temp >>= 1;
    SPLIT_MEMORY_BITS++;
  }

  allocateSplitChunk(0); // first chunk is for core runtime, static, stack, etc., always must be initialized

  // support HEAP8.subarray etc.
  var SHIFT_TABLE = [0, 0, 1, 0, 2, 0, 0, 0, 3];
  function fake(real) {
    var bytes = real[0].BYTES_PER_ELEMENT;
    var shifts = SHIFT_TABLE[bytes];
#if ASSERTIONS
    assert(shifts > 0 || bytes == 1);
#endif
    var that = {
      BYTES_PER_ELEMENT: bytes,
      set: function(array, offset) {
        if (offset === undefined) offset = 0;
        // potentially split over multiple chunks
        while (array.length > 0) {
          var chunk = offset >> SPLIT_MEMORY_BITS;
          var relative = offset & SPLIT_MEMORY_MASK;
          if (relative + (array.length << shifts) < SPLIT_MEMORY) {
            real[chunk].set(array, relative); // all fits in this chunk
            break;
          } else {
            var currSize = SPLIT_MEMORY - relative;
#if ASSERTIONS
            assert(currSize % that.BYTES_PER_ELEMENT === 0);
#endif
            var lastIndex = currSize >> shifts;
            real[chunk].set(array.subarray(0, lastIndex), relative);
            // increments
            array = array.subarray(lastIndex);
            offset += currSize;
          }
        }
      },
      subarray: function(from, to) {
        from = from << shifts;
        var start = from >> SPLIT_MEMORY_BITS;
        if (to === undefined) {
          to = (start + 1) << SPLIT_MEMORY_BITS;
        } else {
          to = to << shifts;
        }
        to = Math.max(from, to); // if to is smaller, we'll get nothing anyway, same as to == from
        if (from < to) {
          var end = (to - 1) >> SPLIT_MEMORY_BITS; // -1, since we do not actually read the last address
#if ASSERTIONS
          assert(start === end, 'subarray cannot span split chunks');
#endif
        }
        if (to > from && (to & SPLIT_MEMORY_MASK) == 0) {
          // avoid the mask on the next line giving 0 for the end
          return real[start].subarray((from & SPLIT_MEMORY_MASK) >> shifts); // just return to the end of the chunk
        }
        return real[start].subarray((from & SPLIT_MEMORY_MASK) >> shifts, (to & SPLIT_MEMORY_MASK) >> shifts);
      },
      buffer: {
        slice: function(from, to) {
#if ASSERTIONS
          assert(to, 'TODO: this is an actual copy, so we could support a slice across multiple chunks');
#endif
          return new Uint8Array(HEAPU8.subarray(from, to)).buffer;
        },
      },
    };
    return that;
  }
  HEAP8 = fake(HEAP8s);
  HEAP16 = fake(HEAP16s);
  HEAP32 = fake(HEAP32s);
  HEAPU8 = fake(HEAPU8s);
  HEAPU16 = fake(HEAPU16s);
  HEAPU32 = fake(HEAPU32s);
  HEAPF32 = fake(HEAPF32s);
  HEAPF64 = fake(HEAPF64s);
})();

#if SAFE_SPLIT_MEMORY
function checkPtr(ptr, shifts) {
  if (ptr <= 0) abort('segmentation fault storing to address ' + ptr);
  if (ptr !== ((ptr >> shifts) << shifts)) abort('alignment error storing to address ' + ptr + ', which was expected to be aligned to a shift of ' + shifts);
  if ((ptr >> SPLIT_MEMORY_BITS) !== (ptr + Math.pow(2, shifts) - 1 >> SPLIT_MEMORY_BITS)) abort('segmentation fault, write spans split chunks ' + [ptr, shifts]);
}
#endif

function get8(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  return HEAP8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] | 0;
}
function get16(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  return HEAP16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] | 0;
}
function get32(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  return HEAP32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] | 0;
}
function getU8(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  return HEAPU8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] | 0;
}
function getU16(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  return HEAPU16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] | 0;
}
function getU32(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  return HEAPU32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] >>> 0;
}
function getF32(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  return +HEAPF32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2];
}
function getF64(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 3);
#endif
  return +HEAPF64s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 3];
}
function set8(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  HEAP8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] = value;
}
function set16(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  HEAP16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] = value;
}
function set32(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  HEAP32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setU8(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  HEAPU8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] = value;
}
function setU16(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  HEAPU16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] = value;
}
function setU32(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  HEAPU32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setF32(ptr, value) {
  ptr = ptr | 0;
  value = +value;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  HEAPF32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setF64(ptr, value) {
  ptr = ptr | 0;
  value = +value;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 3);
#endif
  HEAPF64s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 3] = value;
}
#endif // SPLIT_MEMORY

#endif // USE_PTHREADS

// Endianness check (note: assumes compiler arch was little-endian)
#if SAFE_SPLIT_MEMORY == 0
#if STACK_OVERFLOW_CHECK
#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  HEAP32[0] = 0x63736d65; /* 'emsc' */
#if USE_PTHREADS
} else {
  if (HEAP32[0] !== 0x63736d65) throw 'Runtime error: The application has corrupted its heap memory area (address zero)!';
}
#endif // USE_PTHREADS
#endif // STACK_OVERFLOW_CHECK
#if ASSERTIONS
HEAP16[1] = 0x6373;
if (HEAPU8[2] !== 0x73 || HEAPU8[3] !== 0x63) throw 'Runtime error: expected the system to be little-endian!';
#endif // ASSERTIONS
#endif // SAFE_SPLIT_MEMORY == 0

function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    if (typeof callback == 'function') {
      callback();
      continue;
    }
    var func = callback.func;
    if (typeof func === 'number') {
      if (callback.arg === undefined) {
        Module['dynCall_v'](func);
      } else {
        Module['dynCall_vi'](func, callback.arg);
      }
    } else {
      func(callback.arg === undefined ? null : callback.arg);
    }
  }
}

var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
var __ATMAIN__    = []; // functions called when main() is to be run
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the main() is called

var runtimeInitialized = false;
var runtimeExited = false;

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) runtimeInitialized = true; // The runtime is hosted in the main thread, and bits shared to pthreads via SharedArrayBuffer. No need to init again in pthread.
#endif

function preRun() {
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  // compatibility - merge in anything from Module['preRun'] at this time
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  callRuntimeCallbacks(__ATPRERUN__);
}

function ensureInitRuntime() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  if (runtimeInitialized) return;
  runtimeInitialized = true;
#if USE_PTHREADS
  // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
  __register_pthread_ptr(PThread.mainThreadBlock, /*isMainBrowserThread=*/!ENVIRONMENT_IS_WORKER, /*isMainRuntimeThread=*/1);
#endif
  callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  callRuntimeCallbacks(__ATEXIT__);
  runtimeExited = true;
}

function postRun() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  // compatibility - merge in anything from Module['postRun'] at this time
  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
  callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
  __ATPRERUN__.unshift(cb);
}

function addOnInit(cb) {
  __ATINIT__.unshift(cb);
}

function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}

function addOnExit(cb) {
  __ATEXIT__.unshift(cb);
}

function addOnPostRun(cb) {
  __ATPOSTRUN__.unshift(cb);
}


#if LEGACY_VM_SUPPORT
// check for imul support, and also for correctness ( https://bugs.webkit.org/show_bug.cgi?id=126345 )
if (!Math['imul'] || Math['imul'](0xffffffff, 5) !== -5) Math['imul'] = function imul(a, b) {
  var ah  = a >>> 16;
  var al = a & 0xffff;
  var bh  = b >>> 16;
  var bl = b & 0xffff;
  return (al*bl + ((ah*bl + al*bh) << 16))|0;
};
Math.imul = Math['imul'];

#if PRECISE_F32
#if PRECISE_F32 == 1
if (!Math['fround']) {
  var froundBuffer = new Float32Array(1);
  Math['fround'] = function(x) { froundBuffer[0] = x; return froundBuffer[0] };
}
#else // 2
if (!Math['fround']) Math['fround'] = function(x) { return x };
#endif
Math.fround = Math['fround'];
#else
#if SIMD
if (!Math['fround']) Math['fround'] = function(x) { return x };
#endif
#endif

if (!Math['clz32']) Math['clz32'] = function(x) {
  x = x >>> 0;
  for (var i = 0; i < 32; i++) {
    if (x & (1 << (31 - i))) return i;
  }
  return 32;
};
Math.clz32 = Math['clz32']

if (!Math['trunc']) Math['trunc'] = function(x) {
  return x < 0 ? Math.ceil(x) : Math.floor(x);
};
Math.trunc = Math['trunc'];
#else // LEGACY_VM_SUPPORT
#if ASSERTIONS
assert(Math['imul'] && Math['fround'] && Math['clz32'] && Math['trunc'], 'this is a legacy browser, build with LEGACY_VM_SUPPORT');
#endif
#endif // LEGACY_VM_SUPPORT

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// PRE_RUN_ADDITIONS (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
#if ASSERTIONS
var runDependencyTracking = {};
#endif

function getUniqueRunDependency(id) {
#if ASSERTIONS
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
#endif
  return id;
}

function addRunDependency(id) {
#if USE_PTHREADS
  // We should never get here in pthreads (could no-op this out if called in pthreads, but that might indicate a bug in caller side,
  // so good to be very explicit)
  assert(!ENVIRONMENT_IS_PTHREAD);
#endif
  runDependencies++;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
#if ASSERTIONS
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval !== 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(function() {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err('dependency: ' + dep);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
#endif
}

function removeRunDependency(id) {
  runDependencies--;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
#if ASSERTIONS
  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
#endif
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data

#if PGO
var PGOMonitor = {
  called: {},
  dump: function() {
    var dead = [];
    for (var i = 0; i < this.allGenerated.length; i++) {
      var func = this.allGenerated[i];
      if (!this.called[func]) dead.push(func);
    }
    out('-s DEAD_FUNCTIONS=\'' + JSON.stringify(dead) + '\'\n');
  }
};
Module['PGOMonitor'] = PGOMonitor;
__ATEXIT__.push(function() { PGOMonitor.dump() });
addOnPreRun(function() { addRunDependency('pgo') });
#endif

#if RELOCATABLE
{{{
(function() {
  // add in RUNTIME_LINKED_LIBS, if provided
  if (RUNTIME_LINKED_LIBS.length > 0) {
    return "if (!Module['dynamicLibraries']) Module['dynamicLibraries'] = [];\n" +
           "Module['dynamicLibraries'] = " + JSON.stringify(RUNTIME_LINKED_LIBS) + ".concat(Module['dynamicLibraries']);\n";
  }
  return '';
})()
}}}

addOnPreRun(function() {
  function loadDynamicLibraries(libs) {
    if (libs) {
      libs.forEach(function(lib) {
        loadDynamicLibrary(lib);
      });
    }
    if (Module['asm']['runPostSets']) {
      Module['asm']['runPostSets']();
    }
  }
  // if we can load dynamic libraries synchronously, do so, otherwise, preload
#if WASM
  if (Module['dynamicLibraries'] && Module['dynamicLibraries'].length > 0 && !Module['readBinary']) {
    // we can't read binary data synchronously, so preload
    addRunDependency('preload_dynamicLibraries');
    var binaries = [];
    Module['dynamicLibraries'].forEach(function(lib) {
      fetch(lib, { credentials: 'same-origin' }).then(function(response) {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + lib + "'";
        }
        return response['arrayBuffer']();
      }).then(function(buffer) {
        var binary = new Uint8Array(buffer);
        binaries.push(binary);
        if (binaries.length === Module['dynamicLibraries'].length) {
          // we got them all, wonderful
          loadDynamicLibraries(binaries);
          removeRunDependency('preload_dynamicLibraries');
        }
      });
    });
    return;
  }
#endif
  loadDynamicLibraries(Module['dynamicLibraries']);
});

#if ASSERTIONS
function lookupSymbol(ptr) { // for a pointer, print out all symbols that resolve to it
  var ret = [];
  for (var i in Module) {
    if (Module[i] === ptr) ret.push(i);
  }
  print(ptr + ' is ' + ret);
}
#endif
#endif

var memoryInitializer = null;

#if USE_PTHREADS
#if PTHREAD_HINT_NUM_CORES < 0
if (!ENVIRONMENT_IS_PTHREAD) addOnPreRun(function() {
  addRunDependency('pthreads_querycores');

  var bg = document.createElement('div');
  bg.style = "position: absolute; top: 0%; left: 0%; width: 100%; height: 100%; background-color: black; z-index:1001; -moz-opacity: 0.8; opacity:.80; filter: alpha(opacity=80);";
  var div = document.createElement('div');
  var default_num_cores = navigator.hardwareConcurrency || 4;
  var hwConcurrency = navigator.hardwareConcurrency ? ("says " + navigator.hardwareConcurrency) : "is not available";
  var html = '<div style="width: 100%; text-align:center;"> Thread setup</div> <br /> Number of logical cores: <input type="number" style="width: 50px;" value="'
    + default_num_cores + '" min="1" max="32" id="thread_setup_num_logical_cores"></input> <br /><span style="font-size: 75%;">(<span style="font-family: monospace;">navigator.hardwareConcurrency</span> '
    + hwConcurrency + ')</span> <br />';
#if PTHREAD_POOL_SIZE < 0
  html += 'PThread pool size: <input type="number" style="width: 50px;" value="'
    + default_num_cores + '" min="1" max="32" id="thread_setup_pthread_pool_size"></input> <br />';
#endif
  html += ' <br /> <input type="button" id="thread_setup_button_go" value="Go"></input>';
  div.innerHTML = html;
  div.style = 'position: absolute; top: 35%; left: 35%; width: 30%; height: 150px; padding: 16px; border: 16px solid gray; background-color: white; z-index:1002; overflow: auto;';
  document.body.appendChild(bg);
  document.body.appendChild(div);
  var goButton = document.getElementById('thread_setup_button_go');
  goButton.onclick = function() {
    var num_logical_cores = parseInt(document.getElementById('thread_setup_num_logical_cores').value);
    _emscripten_force_num_logical_cores(num_logical_cores);
#if PTHREAD_POOL_SIZE < 0
    var pthread_pool_size = parseInt(document.getElementById('thread_setup_pthread_pool_size').value);
    PThread.allocateUnusedWorkers(pthread_pool_size, function() { removeRunDependency('pthreads_querycores'); });
#else
    removeRunDependency('pthreads_querycores');
#endif
    document.body.removeChild(bg);
    document.body.removeChild(div);
  }
});
#endif
#endif

#if PTHREAD_POOL_SIZE > 0
// To work around https://bugzilla.mozilla.org/show_bug.cgi?id=1049079, warm up a worker pool before starting up the application.
if (!ENVIRONMENT_IS_PTHREAD) addOnPreRun(function() { if (typeof SharedArrayBuffer !== 'undefined') { addRunDependency('pthreads'); PThread.allocateUnusedWorkers({{{PTHREAD_POOL_SIZE}}}, function() { removeRunDependency('pthreads'); }); }});
#endif

#if ASSERTIONS
#if NO_FILESYSTEM
var /* show errors on likely calls to FS when it was not included */ FS = {
  error: function() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1');
  },
  init: function() { FS.error() },
  createDataFile: function() { FS.error() },
  createPreloadedFile: function() { FS.error() },
  createLazyFile: function() { FS.error() },
  open: function() { FS.error() },
  mkdev: function() { FS.error() },
  registerDevice: function() { FS.error() },
  analyzePath: function() { FS.error() },
  loadFilesFromDB: function() { FS.error() },

  ErrnoError: function ErrnoError() { FS.error() },
};
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;
#endif
#endif

#if CYBERDWARF
var cyberDWARFFile = '{{{ BUNDLED_CD_DEBUG_FILE }}}';
#endif

#include "URIUtils.js"

#if WASM
function integrateWasmJS() {
  // wasm.js has several methods for creating the compiled code module here:
  //  * 'native-wasm' : use native WebAssembly support in the browser
  //  * 'interpret-s-expr': load s-expression code from a .wast and interpret
  //  * 'interpret-binary': load binary wasm and interpret
  //  * 'interpret-asm2wasm': load asm.js code, translate to wasm, and interpret
  //  * 'asmjs': no wasm, just load the asm.js code and use that (good for testing)
  // The method is set at compile time (BINARYEN_METHOD)
  // The method can be a comma-separated list, in which case, we will try the
  // options one by one. Some of them can fail gracefully, and then we can try
  // the next.

  // inputs

  var method = '{{{ BINARYEN_METHOD }}}';

  var wasmTextFile = '{{{ WASM_TEXT_FILE }}}';
  var wasmBinaryFile = '{{{ WASM_BINARY_FILE }}}';
  var asmjsCodeFile = '{{{ ASMJS_CODE_FILE }}}';

  if (typeof Module['locateFile'] === 'function') {
    if (!isDataURI(wasmTextFile)) {
      wasmTextFile = Module['locateFile'](wasmTextFile);
    }
    if (!isDataURI(wasmBinaryFile)) {
      wasmBinaryFile = Module['locateFile'](wasmBinaryFile);
    }
    if (!isDataURI(asmjsCodeFile)) {
      asmjsCodeFile = Module['locateFile'](asmjsCodeFile);
    }
  }

  // utilities

  var wasmPageSize = 64*1024;

  var info = {
    'global': null,
    'env': null,
    'asm2wasm': asm2wasmImports,
    'parent': Module // Module inside wasm-js.cpp refers to wasm-js.cpp; this allows access to the outside program.
  };

  var exports = null;

#if BINARYEN_METHOD != 'native-wasm'
  function lookupImport(mod, base) {
    var lookup = info;
    if (mod.indexOf('.') < 0) {
      lookup = (lookup || {})[mod];
    } else {
      var parts = mod.split('.');
      lookup = (lookup || {})[parts[0]];
      lookup = (lookup || {})[parts[1]];
    }
    if (base) {
      lookup = (lookup || {})[base];
    }
    if (lookup === undefined) {
      abort('bad lookupImport to (' + mod + ').' + base);
    }
    return lookup;
  }
#endif // BINARYEN_METHOD != 'native-wasm'

  function mergeMemory(newBuffer) {
    // The wasm instance creates its memory. But static init code might have written to
    // buffer already, including the mem init file, and we must copy it over in a proper merge.
    // TODO: avoid this copy, by avoiding such static init writes
    // TODO: in shorter term, just copy up to the last static init write
    var oldBuffer = Module['buffer'];
    if (newBuffer.byteLength < oldBuffer.byteLength) {
      err('the new buffer in mergeMemory is smaller than the previous one. in native wasm, we should grow memory here');
    }
    var oldView = new Int8Array(oldBuffer);
    var newView = new Int8Array(newBuffer);

#if MEM_INIT_IN_WASM == 0
    // If we have a mem init file, do not trample it
    if (!memoryInitializer) {
      oldView.set(newView.subarray(Module['STATIC_BASE'], Module['STATIC_BASE'] + Module['STATIC_BUMP']), Module['STATIC_BASE']);
    }
#endif

    newView.set(oldView);
    updateGlobalBuffer(newBuffer);
    updateGlobalBufferViews();
  }

  function fixImports(imports) {
#if WASM_BACKEND
    var ret = {};
    for (var i in imports) {
      var fixed = i;
      if (fixed[0] == '_') fixed = fixed.substr(1);
      ret[fixed] = imports[i];
    }
    return ret;
#else
    return imports;
#endif // WASM_BACKEND
  }

  function getBinary() {
    try {
      if (Module['wasmBinary']) {
        return new Uint8Array(Module['wasmBinary']);
      }
#if SUPPORT_BASE64_EMBEDDING
      var binary = tryParseAsDataURI(wasmBinaryFile);
      if (binary) {
        return binary;
      }
#endif
      if (Module['readBinary']) {
        return Module['readBinary'](wasmBinaryFile);
      } else {
        throw "on the web, we need the wasm binary to be preloaded and set on Module['wasmBinary']. emcc.py will do that for you when generating HTML (but not JS)";
      }
    }
    catch (err) {
      abort(err);
    }
  }

  function getBinaryPromise() {
    // if we don't have the binary yet, and have the Fetch api, use that
    // in some environments, like Electron's render process, Fetch api may be present, but have a different context than expected, let's only use it on the Web
    if (!Module['wasmBinary'] && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && typeof fetch === 'function') {
      return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function(response) {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
        }
        return response['arrayBuffer']();
      }).catch(function () {
        return getBinary();
      });
    }
    // Otherwise, getBinary should be able to get it synchronously
    return new Promise(function(resolve, reject) {
      resolve(getBinary());
    });
  }

  // do-method functions

#if BINARYEN_METHOD != 'native-wasm'
  function doJustAsm(global, env, providedBuffer) {
    // if no Module.asm, or it's the method handler helper (see below), then apply
    // the asmjs
    if (typeof Module['asm'] !== 'function' || Module['asm'] === methodHandler) {
      if (!Module['asmPreload']) {
        // you can load the .asm.js file before this, to avoid this sync xhr and eval
        {{{ makeEval("eval(Module['read'](asmjsCodeFile));") }}} // set Module.asm
      } else {
        Module['asm'] = Module['asmPreload'];
      }
    }
    if (typeof Module['asm'] !== 'function') {
      err('asm evalling did not set the module properly');
      return false;
    }
    return Module['asm'](global, env, providedBuffer);
  }
#endif // BINARYEN_METHOD != 'native-wasm'

  function doNativeWasm(global, env, providedBuffer) {
    if (typeof WebAssembly !== 'object') {
#if BINARYEN_METHOD == 'native-wasm'
#if ASSERTIONS
      // when the method is just native-wasm, our error message can be very specific
      abort('No WebAssembly support found. Build with -s WASM=0 to target JavaScript instead.');
#endif
#endif
      err('no native wasm support detected');
      return false;
    }
    // prepare memory import
    if (!(Module['wasmMemory'] instanceof WebAssembly.Memory)) {
      err('no native wasm Memory in use');
      return false;
    }
    env['memory'] = Module['wasmMemory'];
    // Load the wasm module and create an instance of using native support in the JS engine.
    info['global'] = {
      'NaN': NaN,
      'Infinity': Infinity
    };
    info['global.Math'] = Math;
    info['env'] = env;
    // handle a generated wasm instance, receiving its exports and
    // performing other necessary setup
    function receiveInstance(instance, module) {
      exports = instance.exports;
      if (exports.memory) mergeMemory(exports.memory);
      Module['asm'] = exports;
      Module["usingWasm"] = true;
#if WASM_BACKEND
      // wasm backend stack goes down
      STACKTOP = STACK_BASE + TOTAL_STACK;
      STACK_MAX = STACK_BASE;
      // can't call stackRestore() here since this function can be called
      // synchronously before stackRestore() is declared.
      Module["asm"]["stackRestore"](STACKTOP);
#endif
#if USE_PTHREADS
      // Keep a reference to the compiled module so we can post it to the workers.
      Module['wasmModule'] = module;
      // Instantiation is synchronous in pthreads and we assert on run dependencies.
      if(!ENVIRONMENT_IS_PTHREAD) removeRunDependency('wasm-instantiate');
#else
      removeRunDependency('wasm-instantiate');
#endif
    }
#if USE_PTHREADS
    if (!ENVIRONMENT_IS_PTHREAD) {
      addRunDependency('wasm-instantiate'); // we can't run yet (except in a pthread, where we have a custom sync instantiator)
    }
#else
    addRunDependency('wasm-instantiate');
#endif

    // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
    // to manually instantiate the Wasm module themselves. This allows pages to run the instantiation parallel
    // to any other async startup actions they are performing.
    if (Module['instantiateWasm']) {
      try {
        return Module['instantiateWasm'](info, receiveInstance);
      } catch(e) {
        err('Module.instantiateWasm callback failed with error: ' + e);
        return false;
      }
    }

#if BINARYEN_ASYNC_COMPILATION
#if RUNTIME_LOGGING
    err('asynchronously preparing wasm');
#endif
#if ASSERTIONS
    // Async compilation can be confusing when an error on the page overwrites Module
    // (for example, if the order of elements is wrong, and the one defining Module is
    // later), so we save Module and check it later.
    var trueModule = Module;
#endif
    function receiveInstantiatedSource(output) {
      // 'output' is a WebAssemblyInstantiatedSource object which has both the module and instance.
      // receiveInstance() will swap in the exports (to Module.asm) so they can be called
#if ASSERTIONS
      assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
      trueModule = null;
#endif
      receiveInstance(output['instance'], output['module']);
    }
    function instantiateArrayBuffer(receiver) {
      getBinaryPromise().then(function(binary) {
        return WebAssembly.instantiate(binary, info);
      }).then(receiver).catch(function(reason) {
        err('failed to asynchronously prepare wasm: ' + reason);
        abort(reason);
      });
    }
    // Prefer streaming instantiation if available.
    if (!Module['wasmBinary'] &&
        typeof WebAssembly.instantiateStreaming === 'function' &&
        !isDataURI(wasmBinaryFile) &&
        typeof fetch === 'function') {
      WebAssembly.instantiateStreaming(fetch(wasmBinaryFile, { credentials: 'same-origin' }), info)
        .then(receiveInstantiatedSource)
        .catch(function(reason) {
          // We expect the most common failure cause to be a bad MIME type for the binary,
          // in which case falling back to ArrayBuffer instantiation should work.
          err('wasm streaming compile failed: ' + reason);
          err('falling back to ArrayBuffer instantiation');
          instantiateArrayBuffer(receiveInstantiatedSource);
        });
    } else {
      instantiateArrayBuffer(receiveInstantiatedSource);
    }
    return {}; // no exports yet; we'll fill them in later
#else
    var instance;
    try {
      instance = new WebAssembly.Instance(new WebAssembly.Module(getBinary()), info)
    } catch (e) {
      err('failed to compile wasm module: ' + e);
      if (e.toString().indexOf('imported Memory with incompatible size') >= 0) {
        err('Memory size incompatibility issues may be due to changing TOTAL_MEMORY at runtime to something too large. Use ALLOW_MEMORY_GROWTH to allow any size memory (and also make sure not to set TOTAL_MEMORY at runtime to something smaller than it was at compile time).');
      }
      return false;
    }
    receiveInstance(instance);
    return exports;
#endif
  }

#if BINARYEN_METHOD != 'native-wasm'
  function doWasmPolyfill(global, env, providedBuffer, method) {
    if (typeof WasmJS !== 'function') {
      err('WasmJS not detected - polyfill not bundled?');
      return false;
    }

    // Use wasm.js to polyfill and execute code in a wasm interpreter.
    var wasmJS = WasmJS({});

    // XXX don't be confused. Module here is in the outside program. wasmJS is the inner wasm-js.cpp.
    wasmJS['outside'] = Module; // Inside wasm-js.cpp, Module['outside'] reaches the outside module.

    // Information for the instance of the module.
    wasmJS['info'] = info;

    wasmJS['lookupImport'] = lookupImport;

    assert(providedBuffer === Module['buffer']); // we should not even need to pass it as a 3rd arg for wasm, but that's the asm.js way.

    info.global = global;
    info.env = env;

    // polyfill interpreter expects an ArrayBuffer
    assert(providedBuffer === Module['buffer']);
    env['memory'] = providedBuffer;
    assert(env['memory'] instanceof ArrayBuffer);

    wasmJS['providedTotalMemory'] = Module['buffer'].byteLength;

    // Prepare to generate wasm, using either asm2wasm or s-exprs
    var code;
    if (method === 'interpret-binary') {
      code = getBinary();
    } else {
      code = Module['read'](method == 'interpret-asm2wasm' ? asmjsCodeFile : wasmTextFile);
    }
    var temp;
    if (method == 'interpret-asm2wasm') {
      temp = wasmJS['_malloc'](code.length + 1);
      wasmJS['writeAsciiToMemory'](code, temp);
      wasmJS['_load_asm2wasm'](temp);
    } else if (method === 'interpret-s-expr') {
      temp = wasmJS['_malloc'](code.length + 1);
      wasmJS['writeAsciiToMemory'](code, temp);
      wasmJS['_load_s_expr2wasm'](temp);
    } else if (method === 'interpret-binary') {
      temp = wasmJS['_malloc'](code.length);
      wasmJS['HEAPU8'].set(code, temp);
      wasmJS['_load_binary2wasm'](temp, code.length);
    } else {
      throw 'what? ' + method;
    }
    wasmJS['_free'](temp);

    wasmJS['_instantiate'](temp);

    if (Module['newBuffer']) {
      mergeMemory(Module['newBuffer']);
      Module['newBuffer'] = null;
    }

    exports = wasmJS['asmExports'];

    return exports;
  }
#endif // BINARYEN_METHOD != 'native-wasm'

  // We may have a preloaded value in Module.asm, save it
  Module['asmPreload'] = Module['asm'];

  // Memory growth integration code

  var asmjsReallocBuffer = Module['reallocBuffer'];

  var wasmReallocBuffer = function(size) {
    var PAGE_MULTIPLE = Module["usingWasm"] ? WASM_PAGE_SIZE : ASMJS_PAGE_SIZE; // In wasm, heap size must be a multiple of 64KB. In asm.js, they need to be multiples of 16MB.
    size = alignUp(size, PAGE_MULTIPLE); // round up to wasm page size
    var old = Module['buffer'];
    var oldSize = old.byteLength;
    if (Module["usingWasm"]) {
      // native wasm support
      try {
        var result = Module['wasmMemory'].grow((size - oldSize) / wasmPageSize); // .grow() takes a delta compared to the previous size
        if (result !== (-1 | 0)) {
          // success in native wasm memory growth, get the buffer from the memory
          return Module['buffer'] = Module['wasmMemory'].buffer;
        } else {
          return null;
        }
      } catch(e) {
#if ASSERTIONS
        console.error('Module.reallocBuffer: Attempted to grow from ' + oldSize  + ' bytes to ' + size + ' bytes, but got error: ' + e);
#endif
        return null;
      }
    }
#if BINARYEN_METHOD != 'native-wasm'
    else {
      // wasm interpreter support
      exports['__growWasmMemory']((size - oldSize) / wasmPageSize); // tiny wasm method that just does grow_memory
      // in interpreter, we replace Module.buffer if we allocate
      return Module['buffer'] !== old ? Module['buffer'] : null; // if it was reallocated, it changed
    }
#endif // BINARYEN_METHOD != 'native-wasm'
  };

  Module['reallocBuffer'] = function(size) {
    if (finalMethod === 'asmjs') {
      return asmjsReallocBuffer(size);
    } else {
      return wasmReallocBuffer(size);
    }
  };

  // we may try more than one; this is the final one, that worked and we are using
  var finalMethod = '';

  // Provide an "asm.js function" for the application, called to "link" the asm.js module. We instantiate
  // the wasm module at that time, and it receives imports and provides exports and so forth, the app
  // doesn't need to care that it is wasm or olyfilled wasm or asm.js.

  Module['asm'] = function(global, env, providedBuffer) {
#if BINARYEN_METHOD != 'native-wasm'
    global = fixImports(global);
#endif
    env = fixImports(env);

    // import table
    if (!env['table']) {
      var TABLE_SIZE = Module['wasmTableSize'];
      if (TABLE_SIZE === undefined) TABLE_SIZE = 1024; // works in binaryen interpreter at least
      var MAX_TABLE_SIZE = Module['wasmMaxTableSize'];
      if (typeof WebAssembly === 'object' && typeof WebAssembly.Table === 'function') {
        if (MAX_TABLE_SIZE !== undefined) {
          env['table'] = new WebAssembly.Table({ 'initial': TABLE_SIZE, 'maximum': MAX_TABLE_SIZE, 'element': 'anyfunc' });
        } else {
          env['table'] = new WebAssembly.Table({ 'initial': TABLE_SIZE, element: 'anyfunc' });
        }
      } else {
        env['table'] = new Array(TABLE_SIZE); // works in binaryen interpreter at least
      }
      Module['wasmTable'] = env['table'];
    }

    if (!env['memoryBase']) {
      env['memoryBase'] = Module['STATIC_BASE']; // tell the memory segments where to place themselves
    }
    if (!env['tableBase']) {
      env['tableBase'] = 0; // table starts at 0 by default, in dynamic linking this will change
    }

    // try the methods. each should return the exports if it succeeded

    var exports;
#if BINARYEN_METHOD == 'native-wasm'
    exports = doNativeWasm(global, env, providedBuffer);
#else // native-wasm
#if BINARYEN_METHOD == 'asmjs'
    exports = doJustAsm(global, env, providedBuffer);
#else
    var methods = method.split(',');

    for (var i = 0; i < methods.length; i++) {
      var curr = methods[i];

#if RUNTIME_LOGGING
      err('trying binaryen method: ' + curr);
#endif

      finalMethod = curr;

      if (curr === 'native-wasm') {
        if (exports = doNativeWasm(global, env, providedBuffer)) break;
      } else if (curr === 'asmjs') {
        if (exports = doJustAsm(global, env, providedBuffer)) break;
      } else if (curr === 'interpret-asm2wasm' || curr === 'interpret-s-expr' || curr === 'interpret-binary') {
        if (exports = doWasmPolyfill(global, env, providedBuffer, curr)) break;
      } else {
        abort('bad method: ' + curr);
      }
    }
#endif // asmjs
#endif // native-wasm

#if ASSERTIONS
    assert(exports, 'no binaryen method succeeded. consider enabling more options, like interpreting, if you want that: https://github.com/kripken/emscripten/wiki/WebAssembly#binaryen-methods');
#else
    assert(exports, 'no binaryen method succeeded.');
#endif

#if RUNTIME_LOGGING
    err('binaryen method succeeded.');
#endif

    return exports;
  };

  var methodHandler = Module['asm']; // note our method handler, as we may modify Module['asm'] later
}

integrateWasmJS();
#endif

// === Body ===
