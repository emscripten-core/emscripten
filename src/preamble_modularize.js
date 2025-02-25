#if !MINIMAL_RUNTIME || PTHREADS
#if EXPORT_ES6
var _scriptName = import.meta.url;
#else
var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
#if ENVIRONMENT_MAY_BE_NODE
if (typeof __filename != 'undefined') _scriptName = _scriptName || __filename;
#endif
#endif
#endif

#if EXPORT_ES6
// Ensure Closure and Acorn (specifically the JSDCE pass) is aware of the export.
// This gets replaced by `export default` in the final output.
// https://stackoverflow.com/questions/46092308
window["{{{ EXPORT_NAME }}}"] = {{{ asyncIf(WASM_ASYNC_COMPILATION || ENVIRONMENT_MAY_BE_NODE) }}}function(moduleArg = {}) {
#else
var {{{ EXPORT_NAME }}} = {{{ asyncIf(WASM_ASYNC_COMPILATION) }}}function(moduleArg = {}) {
#endif

var Module = moduleArg;
