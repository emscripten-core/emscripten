// SOURCE_PHASE_IMPORTS combined with WASM_ESM_INTEGRATION: the glue emits a
// source-phase import for the wasm module alongside the value-phase import that
// binds the wasm exports. emitDCEGraph must skip the source-phase import
// (its specifier has no `imported`) and only treat the value-phase `.wasm`
// import as wasm export bindings.

// Source-phase import of the wasm module itself. This is NOT a wasm export
// binding and must be ignored by the DCE graph builder.
import source wasmModule from './a.out.wasm';

function fd_write_impl() {
}

// wasm exports received as ES imports (value-phase).
import {
  memory,
  main as _main,
  used_export as _used_export,
  unused_export as _unused_export,
} from './a.out.wasm';

export {
  fd_write_impl as fd_write,
};

export { _main };

WebAssembly.instantiate(wasmModule);
_used_export();
