#if ASSERTIONS
// Assertion for attempting to access module properties on the incoming
// moduleArg.  In the past we used this object as the prototype of the module
// and assigned properties to it, but now we return a distinct object.  This
// keeps the instance private until it is ready (i.e the promise has been
// resolved).
for (const prop of Object.keys(Module)) {
  if (!(prop in moduleArg)) {
    Object.defineProperty(moduleArg, prop, {
      configurable: true,
      get() {
        abort(`Access to module property ('${prop}') is no longer possible via the module constructor argument; Instead, use the result of the module constructor.`)
      }
    });
  }
}
#endif

// In MODULARIZE mode we wrap the generated code in a factory function
// and return either the Module itself, or a promise of the module.
if (runtimeInitialized)  {
  return Module;
} else {
  // Set up the promise that indicates the Module is initialized
  return new Promise((resolve, reject) => {
    readyPromiseResolve = resolve;
    readyPromiseReject = reject;
  });
}
}; // End factory function


// Export using a UMD style export
#if !EXPORT_ES6 && !MINIMAL_RUNTIME
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

isPthread && {{{ EXPORT_NAME }}}();

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

isWW && {{{ EXPORT_NAME }}}();

#endif // WASM_WORKERS
