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
 * The closure_compiler() method in tools/building.py refers to this file when calling closure.
 */

// Don't minify createRequire
var createRequire;

/**
 * @const
 * @suppress {duplicate, checkTypes}
 */
var WebAssembly = {};
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

/**
 * This was removed from upstream closure compiler in
 * https://github.com/google/closure-compiler/commit/f83322c1b.
 * Perhaps we should remove it too?
 *
 * TODO(sbc): Remove this once SDL2 is updated not to depend on it.
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

// Common between node-externs and v8-externs
var os = {};

AudioWorkletProcessor.parameterDescriptors;

var scheduler = {};

/** @type {boolean} */
ArrayBuffer.prototype.resizable;

/** @type {boolean} */
SharedArrayBuffer.prototype.growable;
