/**
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * This file contains definitions for things that we'd really rather the closure compiler *didn't* minify.
 * See http://code.google.com/p/closure-compiler/wiki/FAQ#How_do_I_write_an_externs_file
 * See also the discussion here: https://github.com/emscripten-core/emscripten/issues/1979
 *
 * The closure_compiler() method in tools/shared.py refers to this file when calling closure.
 */

// Don't minify createRequire
var createRequire;

// Closure externs used by library_sockfs.js

/**
 * Don't minify Math.*
 */
/**
 * @suppress {duplicate}
 */
var Math = {};
Math.abs = function() {};
Math.cos = function() {};
Math.sin = function() {};
Math.tan = function() {};
Math.acos = function() {};
Math.asin = function() {};
Math.atan = function() {};
Math.atan2 = function() {};
Math.exp = function() {};
Math.log = function() {};
Math.sqrt = function() {};
Math.ceil = function() {};
Math.floor = function() {};
Math.pow = function() {};
Math.imul = function() {};
Math.fround = function() {};
Math.round = function() {};
Math.min = function() {};
Math.max = function() {};
Math.clz32 = function() {};
Math.trunc = function() {};

/**
 * @const
 * @suppress {duplicate, checkTypes}
 */
var WebAssembly = {};
/**
 * @param {!WebAssembly.Tag} tag
 * @param {number} index
 */
WebAssembly.Exception.getArg = function(tag, index) {};
/**
 * @param {!WebAssembly.Tag} tag
 */
WebAssembly.Exception.is = function(tag) {};
/**
 * @type {string}
 */
WebAssembly.Exception.stack;

/**
 * Note: Closure compiler does not support function overloading, omit this overload for now.
 * {function(!WebAssembly.Module, Object=):!Promise<!WebAssembly.Instance>}
 */
/**
 * @returns {ArrayBuffer}
 */
WebAssembly.Memory.prototype.toResizableBuffer = function() {};
/**
 * @param {!Function} func
 * @returns {Function}
 */
WebAssembly.promising = function(func) {};
/**
 * @constructor
 * @param {!Function} func
 */
WebAssembly.Suspending = function(func) {};

/**
 * @record
 */
function FunctionType() {}
/**
 * @type {Array<string>}
 */
FunctionType.prototype.parameters;
/**
 * @type {Array<string>}
 */
FunctionType.prototype.results;

/**
 * @constructor
 * @param {!FunctionType} type
 * @param {!Function} func
 */
WebAssembly.Function = function(type, func) {};
/**
 * @param {Function} func
 * @return {FunctionType}
 */
WebAssembly.Function.type = function(func) {};

/**
 * @suppress {undefinedVars}
 */
var wakaUnknownAfter;
/**
 * @suppress {undefinedVars}
 */
var wakaUnknownBefore;

// Module loaders externs, for AMD etc.

/**
 * @param {Function} wrapper
 */
var define = function (wrapper) {};

/**
 * @type {Worker}
 */
var worker;

/**
 * @param {Object} message
 */
var onmessage = function(message) {};
var onmessageerror = function() {};

/**
 * @param {string} type
 * @param {!Function} listener
 * @param {Object|boolean=} optionsOrUseCapture
 */
var addEventListener = function (type, listener, optionsOrUseCapture) {};

/**
 * @param {string} type
 * @param {!Function} listener
 */
var removeEventListener = function (type, listener) {};

/**
 * @type {Function}
 */
var close;

// Closure run on asm.js uses a hack to execute only on shell code, declare externs needed for it.
/**
 * @suppress {undefinedVars}
 */
var wakaGlobal;
/**
 * @suppress {undefinedVars}
 */
var wakaEnv;
/**
 * @suppress {undefinedVars}
 */
var wakaBuffer;


// Browser externs on global window object.
var pageXOffset;
var pageYOffset;
var innerWidth;
var innerHeight;
var outerWidth;
var outerHeight;
var event;
var devicePixelRatio;

/*
 * Avoid closure minifying anything to "id". See #13965
 */
var id;


// Common between node-externs and v8-externs
var os = {};

AudioWorkletProcessor.parameterDescriptors;

var scheduler = {};

/** @type {boolean} */
ArrayBuffer.prototype.resizable;

/** @type {boolean} */
SharedArrayBuffer.prototype.growable;
