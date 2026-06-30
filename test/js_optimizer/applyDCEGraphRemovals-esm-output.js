function fd_write_impl() {}

function fd_close_impl() {}

function unused_import_impl() {}

import { memory, main as _main, used_export as _used_export, memory as wasmMemory } from "./a.out.wasm";

export { fd_write_impl as fd_write, fd_close_impl as fd_close };

export { _main };
