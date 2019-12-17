// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "Fetch.js"

if (typeof Atomics === 'undefined') {
  // Polyfill singlethreaded atomics ops from http://lars-t-hansen.github.io/ecmascript_sharedmem/shmem.html#Atomics.add
  // No thread-safety needed since we don't have multithreading support.
  Atomics = {};
  Atomics['add'] = function(t, i, v) { var w = t[i]; t[i] += v; return w; }
  Atomics['and'] = function(t, i, v) { var w = t[i]; t[i] &= v; return w; }
  Atomics['compareExchange'] = function(t, i, e, r) { var w = t[i]; if (w == e) t[i] = r; return w; }
  Atomics['exchange'] = function(t, i, v) { var w = t[i]; t[i] = v; return w; }
  Atomics['wait'] = function(t, i, v, o) { if (t[i] != v) return 'not-equal'; else return 'timed-out'; }
  Atomics['notify'] = function(t, i, c) { return 0; }
  Atomics['wakeOrRequeue'] = function(t, i1, c, i2, v) { return 0; }
  Atomics['isLockFree'] = function(s) { return true; }
  Atomics['load'] = function(t, i) { return t[i]; }
  Atomics['or'] = function(t, i, v) { var w = t[i]; t[i] |= v; return w; }
  Atomics['store'] = function(t, i, v) { t[i] = v; return v; }
  Atomics['sub'] = function(t, i, v) { var w = t[i]; t[i] -= v; return w; }
  Atomics['xor'] = function(t, i, v) { var w = t[i]; t[i] ^= v; return w; }
}

var Atomics_add = Atomics.add;
var Atomics_and = Atomics.and;
var Atomics_compareExchange = Atomics.compareExchange;
var Atomics_exchange = Atomics.exchange;
var Atomics_wait = Atomics.wait;
var Atomics_wake = Atomics.wake;
var Atomics_wakeOrRequeue = Atomics.wakeOrRequeue;
var Atomics_isLockFree = Atomics.isLockFree;
var Atomics_load = Atomics.load;
var Atomics_or = Atomics.or;
var Atomics_store = Atomics.store;
var Atomics_sub = Atomics.sub;
var Atomics_xor = Atomics.xor;

function load1(ptr) { return HEAP8[ptr>>2]; }
function store1(ptr, value) { HEAP8[ptr>>2] = value; }
function load2(ptr) { return HEAP16[ptr>>2]; }
function store2(ptr, value) { HEAP16[ptr>>2] = value; }
function load4(ptr) { return HEAP32[ptr>>2]; }
function store4(ptr, value) { HEAP32[ptr>>2] = value; }

var ENVIRONMENT_IS_FETCH_WORKER = true;
var ENVIRONMENT_IS_WORKER = true;
var ENVIRONMENT_IS_PTHREAD = true;
var __pthread_is_main_runtime_thread=0;
var DYNAMICTOP_PTR = 0;
var nan = NaN;
var inf = Infinity;

function _emscripten_asm_const_v() {}

function assert(condition) {
  if (!condition) console.error('assert failure!');
}

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

  var queuedOperations = Atomics_load(HEAPU32, queuePtr >> 2);
  var queueSize = Atomics_load(HEAPU32, queuePtr + 8 >> 2);
  for(var i = 0; i < numQueuedItems; ++i) {
    var fetch = Atomics_load(HEAPU32, (queuedOperations >> 2)+i);
    function successcb(fetch) {
      Atomics.compareExchange(HEAPU32, fetch + {{{ C_STRUCTS.emscripten_fetch_t.__proxyState }}} >> 2, 1, 2);
      Atomics.wake(HEAP32, fetch + {{{ C_STRUCTS.emscripten_fetch_t.__proxyState }}} >> 2, 1);
    }
    function errorcb(fetch) {
      Atomics.compareExchange(HEAPU32, fetch + {{{ C_STRUCTS.emscripten_fetch_t.__proxyState }}} >> 2, 1, 2);
      Atomics.wake(HEAP32, fetch + {{{ C_STRUCTS.emscripten_fetch_t.__proxyState }}} >> 2, 1);
    }
    function progresscb(fetch) {
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
    HEAP8 = new Int8Array(buffer);
    HEAPU8 = new Uint8Array(buffer);
    HEAP16 = new Int16Array(buffer);
    HEAPU16 = new Uint16Array(buffer);
    HEAP32 = new Int32Array(buffer);
    HEAPU32 = new Uint32Array(buffer);
    interval = setInterval(processWorkQueue, 100);
  }
}
