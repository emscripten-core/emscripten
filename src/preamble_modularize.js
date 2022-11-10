/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if SOURCE_PHASE_IMPORTS
import source wasmModule from './{{{ WASM_BINARY_FILE }}}';
#endif

// In EXPORT_ES6 mode we can just use 'import.meta.url'.
// Single threaded MINIMAL_RUNTIME programs do not need access to document.currentScript
#if ENVIRONMENT_MAY_BE_WEB && !EXPORT_ES6 && (!MINIMAL_RUNTIME || PTHREADS)
// When MODULARIZE this JS may be executed later, after 
// document.currentScript is gone, so we save it.
var _scriptName = globalThis.document?.currentScript?.src;
#endif

#if EXPORT_ES6
// Ensure Closure recognizes the export. This gets replaced by
// an `export default ...` declaration in the final output.
// See also: `phase_final_emitting` in link.py and https://stackoverflow.com/questions/46092308.
window["{{{ EXPORT_NAME }}}"] = async function(moduleArg = {}) {
#else
async function {{{ EXPORT_NAME }}}(moduleArg = {}) {
#endif

var Module = moduleArg;
