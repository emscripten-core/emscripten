// WASM_ESM_INTEGRATION: the wasm<->JS boundary is expressed with native ES
// import/export syntax rather than the `wasmImports` object and
// `wasmExports['x']` member uses, and emitDCEGraph must build the same graph
// from it.

// JS functions implementing wasm imports.
function fd_write_impl() {
}
function unused_import_impl() {
}

// A JS function only reachable from a wasm import edge.
function helper() {
}
function fd_close_impl() {
  helper();
}

// wasm exports received as ES imports. `memory` is imported twice (plain and
// aliased) and must map to a single export node.
import {
  memory,
  __indirect_function_table,
  main as _main,
  used_export as _used_export,
  unused_export as _unused_export,
  memory as wasmMemory,
} from './a.out.wasm';

// JS functions exported to the wasm module (the wasm imports).
export {
  fd_write_impl as fd_write,
  fd_close_impl as fd_close,
  unused_import_impl as unused_import,
};

// Re-export of a wasm export to the JS entry point: a top-level use that should
// root the underlying `main` export, not be treated as a wasm import.
export { _main };

// MODULARIZE=instance runtime exports. These are plain JS bindings, not wasm
// import edges, and must be left untouched (bare `export {..}` form).
var HEAP32;
var baz = () => {};
export { HEAP32, baz };

// A reserved-word runtime export (from the reserved-export-names support): the
// local is legalized to `$default` and aliased back to `default`. Though
// aliased like a wasm import edge, a wasm import name can never be `default`,
// so this must be treated as a plain runtime export, not a wasm import.
var $default = {};
export { $default as default };

// Top-level uses: root the memory export (via the alias) and one wasm export.
wasmMemory.buffer;
_used_export();
