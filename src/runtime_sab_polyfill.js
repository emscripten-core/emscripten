#if USE_PTHREADS && !WASM
if (typeof SharedArrayBuffer !== 'undefined') {
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
}

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
#endif
