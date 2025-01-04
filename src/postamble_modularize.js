// In MODULARIZE mode we wrap the generated code in a factory function
// and return either the Module itself, or a promise of the module.
//
// We assign to the `moduleRtn` global here and configure closure to see
// this as and extern so it won't get minified.

#if WASM_ASYNC_COMPILATION

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

#if USE_READY_PROMISE
  return readyPromise;
#else
  return {};
#endif
#else  // WASM_ASYNC_COMPILATION
  return Module;
#endif // WASM_ASYNC_COMPILATION
}; // End factory function

#if ASSERTIONS && MODULARIZE != 'instance'
(() => {
  // Create a small, never-async wrapper around {{{ EXPORT_NAME }}} which
  // checks for callers incorrectly using it with `new`.
  var real_{{{ EXPORT_NAME }}} = {{{ EXPORT_NAME }}};
  {{{ EXPORT_NAME }}} = function(arg) {
    if (new.target) throw new Error("{{{ EXPORT_NAME }}}() should not be called with `new {{{ EXPORT_NAME }}}()`");
    return real_{{{ EXPORT_NAME }}}(arg);
  }
})();
#endif

// Export using a UMD style export, or ES6 exports if selected
#if EXPORT_ES6
#if MODULARIZE == 'instance'
{{{ declareInstanceExports() }}}
#else
export default {{{ EXPORT_NAME }}};
#endif
#else
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = {{{ EXPORT_NAME }}};
  // This default export looks redundant, but it allows TS to import this
  // commonjs style module.
  module.exports.default = {{{ EXPORT_NAME }}};
} else if (typeof define === 'function' && define['amd']) {
  define([], () => {{{ EXPORT_NAME }}});
}
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
var isNode = typeof globalThis.process?.versions?.node == 'string';
if (isNode) isPthread = {{{ nodePthreadDetection() }}};
#endif
#elif ENVIRONMENT_MAY_BE_NODE
var isPthread = {{{ nodePthreadDetection() }}};
#endif ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER

// When running as a pthread, construct a new instance on startup
#if MODULARIZE == 'instance'
isPthread && init();
#else
isPthread && {{{ EXPORT_NAME }}}();
#endif

#endif // PTHREADS
