/**
 * @license
 * Copyright 2025 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// This code implements the `-sMODULARIZE` settings by taking the generated
// JS program code (INNER_JS_CODE) and wrapping it in a factory function.

#if SOURCE_PHASE_IMPORTS
import source wasmModule from './{{{ WASM_BINARY_FILE }}}';
#endif

#if ENVIRONMENT_MAY_BE_WEB && !EXPORT_ES6 && !(MINIMAL_RUNTIME && !PTHREADS)
// Single threaded MINIMAL_RUNTIME programs do not need access to
// document.currentScript, so a simple export declaration is enough.
var {{{ EXPORT_NAME }}} = (() => {
  // When MODULARIZE this JS may be executed later,
  // after document.currentScript is gone, so we save it.
  // In EXPORT_ES6 mode we can just use 'import.meta.url'.
#if MIN_FIREFOX_VERSION < 74 || LEGACY_VM_SUPPORT
  // This modularize.js script is not Babeled, so manually adapt for old browsers.
  var _scriptName = typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined;
#else
  var _scriptName = globalThis.document?.currentScript?.src;
#endif
  return async function(moduleArg = {}) {
    var moduleRtn;

"<<< INNER_JS_CODE >>>"

    return moduleRtn;
  };
})();
#else
// When targeting node and ES6 we use `await import ..` in the generated code
// so the outer function needs to be marked as async.
async function {{{ EXPORT_NAME }}}(moduleArg = {}) {
  var moduleRtn;

"<<< INNER_JS_CODE >>>"

  return moduleRtn;
}
#endif

// Export using a UMD style export, or ES6 exports if selected
#if EXPORT_ES6
export default {{{ EXPORT_NAME }}};
#elif !MINIMAL_RUNTIME
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = {{{ EXPORT_NAME }}};
  // This default export looks redundant, but it allows TS to import this
  // commonjs style module.
  module.exports.default = {{{ EXPORT_NAME }}};
} else if (typeof define === 'function' && define['amd'])
  define([], () => {{{ EXPORT_NAME }}});
#endif

#if PTHREADS

// Create code for detecting if we are running in a pthread.
// Normally this detection is done when the module is itself run but
// when running in MODULARIZE mode we need use this to know if we should
// run the module constructor on startup (true only for pthreads).
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
var isPthread = globalThis.self?.name?.startsWith('em-pthread');
#if ENVIRONMENT_MAY_BE_NODE
// In order to support both web and node we also need to detect node here.
var isNode = {{{ nodeDetectionCode() }}};
if (isNode) isPthread = {{{ nodePthreadDetection() }}}
#endif
#else ENVIRONMENT_MAY_BE_NODE
var isPthread = {{{ nodePthreadDetection() }}}
// When running as a pthread, construct a new instance on startup
#endif

#if MODULARIZE == 'instance'
isPthread && init();
#else
isPthread && {{{ EXPORT_NAME }}}();
#endif

#endif // PTHREADS

#if WASM_WORKERS

// Same as above for for WASM_WORKERS
// Normally this detection is done when the module is itself run but
// when running in MODULARIZE mode we need use this to know if we should
// run the module constructor on startup (true only for pthreads).
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
var isWW = globalThis.self?.name == 'em-ww';
// In order to support both web and node we also need to detect node here.
#if ENVIRONMENT_MAY_BE_NODE
#if !PTHREADS
var isNode = {{{ nodeDetectionCode() }}};
#endif
if (isNode) isWW = {{{ nodeWWDetection() }}};
#endif
#elif ENVIRONMENT_MAY_BE_NODE
var isWW = {{{ nodeWWDetection() }}};
#endif

#if AUDIO_WORKLET
isWW ||= !!globalThis.AudioWorkletGlobalScope;
// When running as a wasm worker, construct a new instance on startup
#endif

#if MODULARIZE == 'instance'
isWW && init();
#else
isWW && {{{ EXPORT_NAME }}}();
#endif

#endif // WASM_WORKERS
