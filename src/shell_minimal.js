/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if MODULARIZE
var Module = moduleArg;
#elif USE_CLOSURE_COMPILER
// if (!Module)` is crucial for Closure Compiler here as it will
// otherwise replace every `Module` occurrence with the object below
var /** @type{Object} */ Module;
if (!Module) /** @suppress{checkTypes}*/Module = 
#if AUDIO_WORKLET
  globalThis.{{{ EXPORT_NAME }}} || 
#endif
  {"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__":1};

#elif ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL

// When running on the web we expect Module to be defined externally, in the
// HTML.  Otherwise we must define it here before its first use
var Module =
#if SUPPORTS_GLOBALTHIS
  // As a small code size optimization, we can use 'globalThis' to refer to the global scope Module variable.
  globalThis.{{{ EXPORT_NAME }}} || {};
#else
  // Otherwise do a good old typeof check.
  typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {};
#endif

#else
var Module = {{{ EXPORT_NAME }}};
#endif

#if MODULARIZE && USE_READY_PROMISE
// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
var readyPromise = new Promise((resolve, reject) => {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});
#if ASSERTIONS
{{{ addReadyPromiseAssertions() }}}
#endif
#endif

#if ENVIRONMENT_MAY_BE_NODE
var ENVIRONMENT_IS_NODE = typeof process == 'object' && process.type != 'renderer';
#endif

#if ENVIRONMENT_MAY_BE_SHELL
var ENVIRONMENT_IS_SHELL = typeof read == 'function';
#endif

#if AUDIO_WORKLET
var ENVIRONMENT_IS_AUDIO_WORKLET = typeof AudioWorkletGlobalScope !== 'undefined';
#endif

#if ASSERTIONS || PTHREADS
#if !ENVIRONMENT_MAY_BE_NODE && !ENVIRONMENT_MAY_BE_SHELL
var ENVIRONMENT_IS_WEB = true
#elif ENVIRONMENT && !ENVIRONMENT.includes(',')
var ENVIRONMENT_IS_WEB = {{{ ENVIRONMENT === 'web' }}};
#elif ENVIRONMENT_MAY_BE_SHELL && ENVIRONMENT_MAY_BE_NODE
var ENVIRONMENT_IS_WEB = !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_SHELL;
#elif ENVIRONMENT_MAY_BE_SHELL
var ENVIRONMENT_IS_WEB = !ENVIRONMENT_IS_SHELL;
#else
var ENVIRONMENT_IS_WEB = !ENVIRONMENT_IS_NODE;
#endif
#endif // ASSERTIONS || PTHREADS

#if WASM_WORKERS
var ENVIRONMENT_IS_WASM_WORKER = Module['$ww'];
#endif

#if ASSERTIONS && ENVIRONMENT_MAY_BE_NODE && ENVIRONMENT_MAY_BE_SHELL
if (ENVIRONMENT_IS_NODE && ENVIRONMENT_IS_SHELL) {
  throw 'unclear environment';
}
#endif

// Redefine these in a --pre-js to override behavior. If you would like to
// remove out() or err() altogether, you can no-op it out to function() {},
// and build with --closure 1 to get Closure optimize out all the uses
// altogether.

#if ENVIRONMENT_MAY_BE_NODE && PTHREADS
// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
// Normally just binding console.log/console.error here works fine, but
// under node (with workers) we see missing/out-of-order messages so route
// directly to stdout and stderr.
// See https://github.com/emscripten-core/emscripten/issues/14804
var defaultPrint = console.log.bind(console);
var defaultPrintErr = console.error.bind(console);
if (ENVIRONMENT_IS_NODE) {
  var fs = require('fs');
  defaultPrint = (...args) => fs.writeSync(1, args.join(' ') + '\n');
  defaultPrintErr = (...args) => fs.writeSync(2, args.join(' ') + '\n');
}
var out = defaultPrint;
var err = defaultPrintErr;
#else
var out = (text) => console.log(text);
var err = (text) => console.error(text);
#endif

// Override this function in a --pre-js file to get a signal for when
// compilation is ready. In that callback, call the function run() to start
// the program.
function ready() {
#if MODULARIZE && USE_READY_PROMISE
  readyPromiseResolve(Module);
#endif // MODULARIZE
#if INVOKE_RUN && HAS_MAIN
  {{{ runIfMainThread("run();") }}}
#elif ASSERTIONS
  out('ready() called, and INVOKE_RUN=0. The runtime is now ready for you to call run() to invoke application _main(). You can also override ready() in a --pre-js file to get this signal as a callback')
#endif
#if PTHREADS
  // This Worker is now ready to host pthreads, tell the main thread we can proceed.
  if (ENVIRONMENT_IS_PTHREAD) {
    startWorker(Module);
  }
#endif
}

#if POLYFILL
// See https://caniuse.com/mdn-javascript_builtins_object_assign
#if MIN_CHROME_VERSION < 45 || MIN_FIREFOX_VERSION < 34 || MIN_SAFARI_VERSION < 90000
#include "polyfill/objassign.js"
#endif
#endif

#if PTHREADS
// MINIMAL_RUNTIME does not support --proxy-to-worker option, so Worker and Pthread environments
// coincide.
var ENVIRONMENT_IS_WORKER = typeof importScripts == 'function';
var ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && self.name?.startsWith('em-pthread');

#if !MODULARIZE
// In MODULARIZE mode _scriptName needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
var _scriptName = (typeof document != 'undefined') ? document.currentScript?.src : undefined;
#endif

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE) {
  var worker_threads = require('worker_threads');
  global.Worker = worker_threads.Worker;
  ENVIRONMENT_IS_WORKER = !worker_threads.isMainThread;
  // Under node we set `workerData` to `em-pthread` to signal that the worker
  // is hosting a pthread.
  ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && worker_threads['workerData'] == 'em-pthread'
  _scriptName = __filename;
} else
#endif
if (ENVIRONMENT_IS_WORKER) {
  _scriptName = self.location.href;
}
#endif // PTHREADS

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
{{{ preJS() }}}

#if !SINGLE_FILE

#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif

#if ENVIRONMENT_MAY_BE_NODE && ((WASM == 1 && !WASM2JS) || WASM == 2)
// Wasm or Wasm2JS loading:

if (ENVIRONMENT_IS_NODE) {
  var fs = require('fs');
#if WASM == 2
  if (typeof WebAssembly != 'undefined') Module['wasm'] = fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.wasm');
  else eval(fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.wasm.js')+'');
#else
#if !WASM2JS
  Module['wasm'] = fs.readFileSync(__dirname + '/{{{ TARGET_BASENAME }}}.wasm');
#endif
#endif
}
#endif

#if ENVIRONMENT_MAY_BE_SHELL && ((WASM == 1 && !WASM2JS) || WASM == 2)
if (ENVIRONMENT_IS_SHELL) {
#if WASM == 2
  if (typeof WebAssembly != 'undefined') Module['wasm'] = read('{{{ TARGET_BASENAME }}}.wasm', 'binary');
  else eval(read('{{{ TARGET_BASENAME }}}.wasm.js')+'');
#else
#if !WASM2JS
  Module['wasm'] = read('{{{ TARGET_BASENAME }}}.wasm', 'binary');
#endif
#endif
}
#endif

#if PTHREADS
}
#endif

#endif // !SINGLE_FILE

