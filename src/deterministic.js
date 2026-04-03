/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var MAGIC = 0;
Math.random = () => {
  MAGIC = Math.pow(MAGIC + 1.8912, 3) % 1;
  return MAGIC;
};

var TIME = 10000;
function deterministicNow() {
  return TIME++;
}

Date.now = deterministicNow;

// Note: this approach does not work on certain versions of Node.js
// Specifically it seems like its not possible to override performance.now on
// node v16 through v18.
// See getPerformanceNow in parseTools.mjs for how we deal with this.
if (globalThis.performance) performance.now = deterministicNow;

Module['thisProgram'] = 'thisProgram'; // for consistency between different builds than between runs of the same build

function hashMemory(id) {
  var ret = 0;
  var len = _sbrk(0);
  for (var i = 0; i < len; i++) {
    ret = (ret*17 + HEAPU8[i])|0;
  }
  return id + ':' + ret;
}

function hashString(s) {
  var ret = 0;
  for (var i = 0; i < s.length; i++) {
    ret = (ret*17 + s.charCodeAt(i))|0;
  }
  return ret;
}

