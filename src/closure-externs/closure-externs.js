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

// Special placeholder for `import.meta` and `await import`.
var EMSCRIPTEN$IMPORT$META;
var EMSCRIPTEN$AWAIT$IMPORT;

// Don't minify createRequire
var createRequire;

// Don't minify startWorker which we use to start workers once the runtime is ready.
/**
 * @param {Object} Module
 */
var startWorker = function(Module) {};

// Closure externs used by library_sockfs.js

/**
 * Backported from latest closure...
 * @see https://developer.mozilla.org/en-US/docs/Web/API/Document/currentScript
 */
Document.prototype.currentScript;

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
 * Atomics
 */

var Atomics = {};
Atomics.compareExchange = function() {};
Atomics.exchange = function() {};
Atomics.wait = function() {};
Atomics.notify = function() {};
Atomics.load = function() {};
Atomics.store = function() {};

/**
 * @const
 * @suppress {duplicate, checkTypes}
 */
var WebAssembly = {};
/**
 * @constructor
 * @param {Object} globalDescriptor
 * @param {*=} value
 */
WebAssembly.Global = function(globalDescriptor, value) {};
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
/** @dict */
WebAssembly.Instance.prototype.exports;
/**
 * @type {!ArrayBuffer}
 */
WebAssembly.Memory.prototype.buffer;
/**
 * @type {number}
 */
WebAssembly.Table.prototype.length;
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
 * AudioWorkletGlobalScope globals
 */
var registerProcessor = function(name, obj) {};
var currentFrame;
var currentTime;
var sampleRate;

/*
 * Avoid closure minifying anything to "id". See #13965
 */
var id;

/**
 * Used in MODULARIZE mode as the name of the incoming module argument.
 * This is generated outside of the code we pass to closure so from closure's
 * POV this is "extern".
 */
var moduleArg;

/**
 * Used in MODULARIZE mode.
 * We need to access this after the code we pass to closure so from closure's
 * POV this is "extern".
 */
var moduleRtn;

/**
 * This was removed from upstream closure compiler in
 * https://github.com/google/closure-compiler/commit/f83322c1b.
 * Perhaps we should remove it do?
 *
 * @param {MediaStreamConstraints} constraints A MediaStreamConstraints object.
 * @param {function(!MediaStream)} successCallback
 *     A NavigatorUserMediaSuccessCallback function.
 * @param {function(!NavigatorUserMediaError)=} errorCallback A
 *     NavigatorUserMediaErrorCallback function.
 * @see http://dev.w3.org/2011/webrtc/editor/getusermedia.html
 * @see https://www.w3.org/TR/mediacapture-streams/
 * @return {undefined}
 */
Navigator.prototype.webkitGetUserMedia = function(
    constraints, successCallback, errorCallback) {};
