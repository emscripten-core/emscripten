#if !MINIMAL_RUNTIME || PTHREADS
#if EXPORT_ES6 && USE_ES6_IMPORT_META
var _scriptName = import.meta.url;
#else
var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
#if ENVIRONMENT_MAY_BE_NODE
if (typeof __filename != 'undefined') _scriptName = _scriptName || __filename;
#endif
#endif
#endif

#if MODULARIZE == 'instance'
export default {{{ asyncIf(WASM_ASYNC_COMPILATION || (EXPORT_ES6 && ENVIRONMENT_MAY_BE_NODE)) }}}function init(moduleArg = {}) {
#else
var {{{ EXPORT_NAME }}} = {{{ asyncIf(WASM_ASYNC_COMPILATION || (EXPORT_ES6 && ENVIRONMENT_MAY_BE_NODE)) }}}function(moduleArg = {}) {
#endif

var Module = moduleArg;
