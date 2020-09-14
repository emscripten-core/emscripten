/**
d
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if USE_CLOSURE_COMPILER
// if (!Module)` is crucial for Closure Compiler here as it will otherwise replace every `Module` occurrence with the object below
var /** @type{Object} */ Module;
if (!Module) /** @suppress{checkTypes}*/Module = {"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__":1};
#else
var Module = {{{ EXPORT_NAME }}};
#endif // USE_CLOSURE_COMPILER

#if MODULARIZE && EXPORT_READY_PROMISE
// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
Module['ready'] = new Promise(function(resolve, reject) {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});
#if ASSERTIONS
{{{ addReadyPromiseAssertions("Module['ready']") }}}
#endif
#endif

#if ENVIRONMENT_MAY_BE_NODE
var ENVIRONMENT_IS_NODE = typeof process === 'object';
#endif

#if ENVIRONMENT_MAY_BE_SHELL
var ENVIRONMENT_IS_SHELL = typeof read === 'function';
#endif

#if ASSERTIONS
#if !ENVIRONMENT_MAY_BE_NODE && !ENVIRONMENT_MAY_BE_SHELL
var ENVIRONMENT_IS_WEB = true
#else
#if ENVIRONMENT && ENVIRONMENT.indexOf(',') < 0
var ENVIRONMENT_IS_WEB = {{{ ENVIRONMENT === 'web' }}};
#else
var ENVIRONMENT_IS_WEB = !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_SHELL;
#endif
#endif
#endif

#if ASSERTIONS && ENVIRONMENT_MAY_BE_NODE && ENVIRONMENT_MAY_BE_SHELL
if (ENVIRONMENT_IS_NODE && ENVIRONMENT_IS_SHELL) {
  throw 'unclear environment';
}
#endif

#if !SINGLE_FILE
#if ENVIRONMENT_MAY_BE_NODE && ((WASM == 1 && (!WASM2JS || !MEM_INIT_IN_WASM)) || WASM == 2)
// Wasm or Wasm2JS loading:

if (ENVIRONMENT_IS_NODE) {
  var fs = require('fs');
#if WASM == 2
  if (typeof WebAssembly !== 'undefined') Module['wasm'] = fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.wasm');
  else eval(fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.wasm.js')+'');
#else
#if !WASM2JS
  Module['wasm'] = fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.wasm');
#endif
#endif
#if MEM_INIT_METHOD == 1 && !MEM_INIT_IN_WASM
  Module['mem'] = fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.mem');
#endif
}
#endif

#if ENVIRONMENT_MAY_BE_SHELL && ((WASM == 1 && (!WASM2JS || !MEM_INIT_IN_WASM)) || WASM == 2)
if (ENVIRONMENT_IS_SHELL) {
#if WASM == 2
  if (typeof WebAssembly !== 'undefined') Module['wasm'] = read('{{{ TARGET_BASENAME }}}.wasm', 'binary');
  else eval(read('{{{ TARGET_BASENAME }}}.wasm.js')+'');
#else
#if !WASM2JS
  Module['wasm'] = read('{{{ TARGET_BASENAME }}}.wasm', 'binary');
#endif
#endif
#if MEM_INIT_METHOD == 1 && !MEM_INIT_IN_WASM
  Module['mem'] = read('{{{ TARGET_BASENAME }}}.mem', 'binary');
#endif
}
#endif

#endif // !SINGLE_FILE

// Redefine these in a --pre-js to override behavior. If you would like to
// remove out() or err() altogether, you can no-op it out to function() {},
// and build with --closure 1 to get Closure optimize out all the uses
// altogether.

function out(text) {
  console.log(text);
}

function err(text) {
  console.error(text);
}

// Override this function in a --pre-js file to get a signal for when
// compilation is ready. In that callback, call the function run() to start
// the program.
function ready() {
#if MODULARIZE && EXPORT_READY_PROMISE
  readyPromiseResolve(Module);
#endif // MODULARIZE
#if INVOKE_RUN && hasExportedFunction('_main')
#if USE_PTHREADS
  if (!ENVIRONMENT_IS_PTHREAD) {
#endif
    run();
#if USE_PTHREADS
  }
#endif
#else
#if ASSERTIONS
  console.log('ready() called, and INVOKE_RUN=0. The runtime is now ready for you to call run() to invoke application _main(). You can also override ready() in a --pre-js file to get this signal as a callback')
#endif
#endif
}

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)

// {{PRE_JSES}}

#if USE_PTHREADS

#if !MODULARIZE
// In MODULARIZE mode _scriptDir needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
var _scriptDir = (typeof document !== 'undefined' && document.currentScript) ? document.currentScript.src : undefined;
#endif

// MINIMAL_RUNTIME does not support --proxy-to-worker option, so Worker and Pthread environments
// coincide.
var ENVIRONMENT_IS_WORKER = ENVIRONMENT_IS_PTHREAD = typeof importScripts === 'function';

#if MODULARIZE
if (ENVIRONMENT_IS_WORKER) {
  var buffer = {{{EXPORT_NAME}}}.buffer;
  var STACK_BASE = {{{EXPORT_NAME}}}.STACK_BASE;
  var STACKTOP = {{{EXPORT_NAME}}}.STACKTOP;
  var STACK_MAX = {{{EXPORT_NAME}}}.STACK_MAX;
}
#endif

var currentScriptUrl = typeof _scriptDir !== 'undefined' ? _scriptDir : ((typeof document !== 'undefined' && document.currentScript) ? document.currentScript.src : undefined);
#endif // USE_PTHREADS

{{BODY}}

// {{MODULE_ADDITIONS}}
