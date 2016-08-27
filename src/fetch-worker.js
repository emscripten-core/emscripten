#include "Fetch.js"

var ENVIRONMENT_IS_FETCH_WORKER = true;
var ENVIRONMENT_IS_WORKER = true;
var ENVIRONMENT_IS_PTHREAD = true;
var __pthread_is_main_runtime_thread=0;
var Atomics_load = Atomics.load;
var Atomics_store = Atomics.store;
var Atomics_exchange = Atomics.exchange;
var Atomics_compareExchange = Atomics.compareExchange;
var Atomics_add = Atomics.add;
var Atomics_sub = Atomics.sub;
var Atomics_and = Atomics.and;
var Atomics_or = Atomics.or;
var Atomics_xor = Atomics.xor;
var DYNAMICTOP_PTR = 0;
var TOTAL_MEMORY = 0;
function enlargeMemory() {
  abort('Cannot enlarge memory arrays, since compiling with pthreads support enabled (-s USE_PTHREADS=1).');
}
var nan = NaN;
var inf = Infinity;

function _emscripten_asm_const_v() {}

function assert(condition) {
  if (!condition) console.error('assert failure!');
}

/// TODO: DO SOMETHING ABOUT ME.
function Pointer_stringify(ptr, /* optional */ length) {
  if (length === 0 || !ptr) return "";
  // TODO: use TextDecoder
  // Find the length, and check for UTF while doing so
  var hasUtf = 0;
  var t;
  var i = 0;
  while (1) {
    t = HEAPU8[(((ptr)+(i))>>0)];
    hasUtf |= t;
    if (t == 0 && !length) break;
    i++;
    if (length && i == length) break;
  }
  if (!length) length = i;

  var ret = "";

  if (hasUtf < 128) {
    var MAX_CHUNK = 1024; // split up into chunks, because .apply on a huge string can overflow the stack
    var curr;
    while (length > 0) {
      curr = String.fromCharCode.apply(String, HEAPU8.subarray(ptr, ptr + Math.min(length, MAX_CHUNK)));
      ret = ret ? ret + curr : curr;
      ptr += MAX_CHUNK;
      length -= MAX_CHUNK;
    }
    return ret;
  }
  return Module['UTF8ToString'](ptr);
}


console.log('fetch worker script loading.');
Fetch.staticInit();

var queuePtr = 0;
var buffer = null;
var STACKTOP = 0;
var STACK_MAX = 0;
var HEAP8 = null;
var HEAPU8 = null;
var HEAP16 = null;
var HEAPU16 = null;
var HEAP32 = null;
var HEAPU32 = null;

function processWorkQueue() {
  if (!queuePtr) return;
  var numQueuedItems = Atomics_load(HEAPU32, queuePtr + 4 >> 2);
  if (numQueuedItems == 0) return;

  console.log('polling work to perform, there are ' + numQueuedItems + ' work items in the queue.');
  var queuedOperations = Atomics_load(HEAPU32, queuePtr >> 2);
  var queueSize = Atomics_load(HEAPU32, queuePtr + 8 >> 2);
  for(var i = 0; i < numQueuedItems; ++i) {
    var fetch = Atomics_load(HEAPU32, (queuedOperations >> 2)+i);
    console.log('processWorkQueue: starting fetch');
    console.log('fetch ptr1 ' + fetch + ', state ' + Atomics_load(HEAPU32, fetch + 108 >> 2));
    function successcb(fetch) {
      console.log('fetch ptr ' + fetch + ', state ' + Atomics_load(HEAPU32, fetch + 108 >> 2));
      console.log('FETCH-WORKER: fetch finished');
      var oldVal = Atomics.compareExchange(HEAPU32, fetch + 108 >> 2, 1, 2);
      console.log('atomics wake ' + (fetch + 108));
      Atomics.wake(HEAP32, fetch + 108 >> 2, 1);
    }
    function errorcb(fetch) {
      console.log('FETCH-WORKER: fetch failed');
    }
    function progresscb(fetch) {
      console.log('FETCH-WORKER: fetch progress..');
    }
    try {
      emscripten_start_fetch(fetch, successcb, errorcb, progresscb);
    } catch(e) {
      console.error(e);
    }
    /*
    if (interval != undefined) {
      clearInterval(interval);
      interval = undefined;
    }
    */
  }
  Atomics_store(HEAPU32, queuePtr + 4 >> 2, 0);
}

interval = 0;
this.onmessage = function(e) {
  if (e.data.cmd == 'init') {
    queuePtr = e.data.queuePtr;
    buffer = e.data.buffer;
    STACKTOP = e.data.STACKTOP;
    STACK_MAX = e.data.STACK_MAX;
    DYNAMICTOP_PTR = e.data.DYNAMICTOP_PTR;
    TOTAL_MEMORY = e.data.TOTAL_MEMORY;
    HEAP8 = new Int8Array(buffer);
    HEAPU8 = new Uint8Array(buffer);
    HEAP16 = new Int16Array(buffer);
    HEAPU16 = new Uint16Array(buffer);
    HEAP32 = new Int32Array(buffer);
    HEAPU32 = new Uint32Array(buffer);
    console.log('fetch: fetch Worker initialized. queue ptr ' + queuePtr + ', heap length: ' + buffer.byteLength);
    interval = setInterval(processWorkQueue, 100);
  }
}
