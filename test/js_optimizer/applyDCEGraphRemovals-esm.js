// WASM_ESM_INTEGRATION: unused wasm imports are dropped from the `export {..}`
// that sends JS functions to the wasm, and unused wasm exports (including
// internal ones like the indirect function table) from the `import {..}` that
// receives them, keeping the two ES module interfaces in sync.

function fd_write_impl() {
}
function fd_close_impl() {
}
function unused_import_impl() {
}

import {
  memory,
  __indirect_function_table,
  main as _main,
  used_export as _used_export,
  unused_export as _unused_export,
  memory as wasmMemory,
} from './a.out.wasm';

export {
  fd_write_impl as fd_write,
  fd_close_impl as fd_close,
  unused_import_impl as unused_import,
};

export { _main };

// MODULARIZE=instance runtime exports (bare form) must be left untouched.
var HEAP32;
export { HEAP32 };

// A reserved-word runtime export (aliased `$default as default`) must never be
// treated as a wasm import and dropped, even though it is aliased.
var $default = {};
export { $default as default };

// EXTRA_INFO: { "unusedImports": ["unused_import"], "unusedExports": ["unused_export", "__indirect_function_table"] }
