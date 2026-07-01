// WASM_ESM_INTEGRATION: minified wasm import/export names are applied to the
// native ES import/export specifiers. Only the wasm-facing name of each
// specifier is renamed; the JS-local binding name is left intact (including the
// unaliased `memory`, whose local side must survive).

function fd_write_impl() {
}
function fd_close_impl() {
}

import {
  memory,
  main as _main,
  malloc as _malloc,
  memory as wasmMemory,
} from './a.out.wasm';

export {
  fd_write_impl as fd_write,
  fd_close_impl as fd_close,
};

// Re-export of a wasm export: the local name (_main) is never in the mapping.
export { _main };

// MODULARIZE=instance runtime export (bare form): never in the mapping.
var HEAP32;
export { HEAP32 };

// EXTRA_INFO: { "mapping": { "main": "b", "malloc": "c", "fd_write": "d", "fd_close": "e" } }
