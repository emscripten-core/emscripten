function fd_write_impl() {}

function fd_close_impl() {}

import { memory, b as _main, c as _malloc, memory as wasmMemory } from "./a.out.wasm";

export { fd_write_impl as d, fd_close_impl as e };

export { _main };
